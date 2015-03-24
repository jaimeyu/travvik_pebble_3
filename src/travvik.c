#include "pebble.h"
#include "nb_selector.h"
#include "util.h"
#include "travvik.h"

//#define DEBUG 
#undef DEBUG 
#ifdef DEBUG 
#define TIMER (1000*5) // 5 seconds
#else
#define TIMER (1000*60)
#endif

static Window *window;
static Window *nb_selector;
static Window *nb_selector_stop;
static NumberWindow *wind_bus_sel;
static NumberWindow *wind_stop_sel;

static TextLayer *layer_destination;
static TextLayer *layer_route;
static TextLayer *layer_eta;
static TextLayer *layer_eta_mins_str;
static TextLayer *layer_gps_str;
static TextLayer *layer_station;
static TextLayer *layer_station_str;
//static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
static uint8_t sync_buffer[128];

static uint8_t direction = 0;
int route = 0;
int stop = 0;
int gps = 0;
static int32_t eta = 0;
static int32_t refresh_count = 3;
static uint8_t busy = 0;

enum TRIP_KEYS {
  KEY_ROUTE = 0,     // TUPLE_INT bus #
  KEY_STOP_NUM = 1,      // TUPLE_INT stop #
  KEY_ETA = 2,      // TUPLE_CSTRING stop name
  KEY_DST = 3, // TUPLE_CSTRING
  KEY_STATION_STR = 4,
  KEY_DIRECTION = 5,
  KEY_GPS= 6,
};

#define STR_FETCHING "Fetching..."



static uint8_t error_flag = 0;
static void 
sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "DictError:%d",dict_error);
  if ( app_message_error == 64){
    //error_flag = 1;
  }
}

void set_eta_layer() {
  static char ab[16];
  if (eta < -1){
    text_layer_set_text(layer_eta, "0");
    return;
  }
  itoa(eta,ab,10);
  text_layer_set_text(layer_eta, ab);
}

typedef struct {

    union {
        uint8_t route:1;
        uint8_t stop:1;
        uint8_t eta:1;
        uint8_t dst:1;
        uint8_t stopstr:1;
        uint8_t direction:1;
        uint8_t rsvd:2;
    };
    uint8_t mask;
} MASK_TUPLE_T;

#define MASK_TUPLE_RDY (0b00111111)
#define MASK_TUPLE_RST (0)

static MASK_TUPLE_T mask_rcv = {.mask = MASK_TUPLE_RST };
static void 
sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  static char nb[16];
  static char sb[16];
  //itoa((int)new_tuple->value,nb,15);

  if (error_flag == 1) return;
  switch (key) {
    case KEY_ROUTE:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: BUS_NB:%d", (int)new_tuple->value->int32);
      route = new_tuple->value->int32;
      itoa((int)new_tuple->value->int32,nb,10);
      text_layer_set_text(layer_route, nb);
      number_window_set_value(wind_bus_sel, (int)new_tuple->value->int32);
      mask_rcv.route = 1;
      break;
    case KEY_STOP_NUM:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: stop_NB:%d", (int)new_tuple->value->int32);
      stop = new_tuple->value->int32;
      itoa((int)new_tuple->value->int32,sb,10);
      text_layer_set_text(layer_station, sb);
      number_window_set_value(wind_stop_sel, (int)new_tuple->value->int32);
      mask_rcv.stop = 1;
      break;
    case KEY_ETA:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: ETA:%d", new_tuple->value->int16);
      eta = new_tuple->value->int16;
      set_eta_layer();
      mask_rcv.eta = 1;
      break;
    case KEY_DST:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: dst:%s",new_tuple->value->cstring);
      text_layer_set_text(layer_destination, new_tuple->value->cstring);
      // make it unbusy.
      mask_rcv.dst = 1;
      break;
    case KEY_DIRECTION:
      mask_rcv.direction = 1;
      break;
    case KEY_GPS:
      gps = new_tuple->value->int16;
      if (gps){
          text_layer_set_text(layer_gps_str, "GPS Tracking");
      } else {
          text_layer_set_text(layer_gps_str, "NO GPS");
      }
      break;
    case KEY_STATION_STR:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: station name:");
      text_layer_set_text(layer_station_str, new_tuple->value->cstring);
      mask_rcv.stopstr= 1;
      break;
    default:
      break;
  }

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "mask_rcv.mask:%X", mask_rcv.mask);
  if (mask_rcv.mask == MASK_TUPLE_RDY){
      vibes_short_pulse();
      mask_rcv.mask = MASK_TUPLE_RST;
      busy = 0;
  }
}

void decrement_eta_handler(void *data){
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Decrementing eta");
  if (--refresh_count >= 0){
    eta--;
    if (eta >= 0){
      set_eta_layer();
    }
    else if (eta == -1){
      set_eta_layer();
      send_cmd();
      refresh_count = 3;
    }
  }
  else{
    send_cmd();
    refresh_count = 3;
  }
  app_timer_register(TIMER, decrement_eta_handler, NULL);
}

void send_cmd(void) {
  // Don't send if we're busy sending a request.
  if (busy != 0){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Still handling old req");
    return;
  }

  error_flag = 0;

  //busy = 1;

  // route = number_window_get_value(wind_bus_sel);
  // stop = number_window_get_value(wind_stop_sel);

  Tuplet value =    TupletInteger(KEY_ROUTE, route);
  Tuplet stopnb =   TupletInteger(KEY_STOP_NUM, stop);
  Tuplet arrival =  TupletInteger(KEY_ETA, 0);
  Tuplet dst =      TupletCString(KEY_DST, STR_FETCHING);
  Tuplet station_str = TupletCString(KEY_DST, STR_FETCHING);
  Tuplet dir = TupletInteger(KEY_DIRECTION, direction); 
  Tuplet tgps= TupletInteger(KEY_GPS, 0); 

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_tuplet(iter, &stopnb);
  dict_write_tuplet(iter, &arrival);
  dict_write_tuplet(iter, &dst);
  dict_write_tuplet(iter, &station_str);
  dict_write_tuplet(iter, &dir);
  dict_write_tuplet(iter, &tgps);
  dict_write_end(iter);

  app_message_outbox_send();
  //busy = 1;
}

static void window_load(Window *window) {
/*
What the output needs to look like

|route|Route name|
    ETA
    ETA
Station
Station name

*/


  Layer *window_layer = window_get_root_layer(window);

  layer_route = text_layer_create(GRect(0, 0, 40, 30));
  text_layer_set_text_color(layer_route, GColorWhite);
  text_layer_set_background_color(layer_route, GColorClear);
  text_layer_set_font(layer_route, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(layer_route, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(layer_route));

  layer_destination = text_layer_create(GRect(35, 0, 144, 30));
  text_layer_set_text_color(layer_destination, GColorBlack);
  text_layer_set_background_color(layer_destination, GColorWhite);
  text_layer_set_font(layer_destination, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(layer_destination, GTextAlignmentLeft);
  text_layer_set_overflow_mode (layer_destination, GTextOverflowModeFill);
  layer_add_child(window_layer, text_layer_get_layer(layer_destination));

  layer_eta = text_layer_create(GRect(0, 30 , 144, 68));
  text_layer_set_text_color(layer_eta, GColorWhite);
  text_layer_set_background_color(layer_eta, GColorClear);
  text_layer_set_font(layer_eta, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(layer_eta, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(layer_eta));

  layer_eta_mins_str  = text_layer_create(GRect(115, 80 , 144, 68));
  text_layer_set_text_color(layer_eta_mins_str, GColorWhite);
  text_layer_set_background_color(layer_eta_mins_str, GColorClear);
  text_layer_set_font(layer_eta_mins_str, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(layer_eta_mins_str, GTextAlignmentLeft);
  text_layer_set_text(layer_eta_mins_str, "mins");
  layer_add_child(window_layer, text_layer_get_layer(layer_eta_mins_str));


  layer_station = text_layer_create(GRect(0, 90, 144, 68));
  text_layer_set_text_color(layer_station, GColorWhite);
  text_layer_set_background_color(layer_station, GColorClear);
  text_layer_set_font(layer_station, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(layer_station, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(layer_station));

  layer_station_str = text_layer_create(GRect(0, 120, 144, 68));
  text_layer_set_text_color(layer_station_str, GColorBlack);
  text_layer_set_background_color(layer_station_str, GColorWhite);
  text_layer_set_font(layer_station_str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(layer_station_str, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(layer_station_str));

  layer_gps_str  = text_layer_create(GRect(50, 100 , 144, 68));
  text_layer_set_text_color(layer_gps_str, GColorWhite);
  text_layer_set_background_color(layer_gps_str, GColorClear);
  text_layer_set_font(layer_gps_str, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(layer_gps_str, GTextAlignmentLeft);
  text_layer_set_text(layer_gps_str, "GPS tracking");
  layer_add_child(window_layer, text_layer_get_layer(layer_gps_str));

  Tuplet bus_values[] = {
    TupletInteger(KEY_ROUTE, -1),
    TupletInteger(KEY_STOP_NUM, -1),
    TupletInteger(KEY_ETA, -1),
    TupletCString(KEY_DST, "  bootup              "),
    TupletCString(KEY_STATION_STR, " none            "),
    TupletInteger(KEY_DIRECTION, 0),
    TupletInteger(KEY_GPS, 0),
  };

    int inbound_size = app_message_inbox_size_maximum();
    int outbound_size = app_message_outbox_size_maximum();
    app_message_open(inbound_size, outbound_size);
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), bus_values, ARRAY_LENGTH(bus_values),
    sync_tuple_changed_callback, sync_error_callback, NULL);

    app_timer_register(TIMER, decrement_eta_handler, NULL);

  //send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }

  text_layer_destroy(layer_route);
  text_layer_destroy(layer_eta);
  text_layer_destroy(layer_eta_mins_str);
  text_layer_destroy(layer_gps_str);
  text_layer_destroy(layer_destination);
  text_layer_destroy(layer_station);
  text_layer_destroy(layer_station_str);
  //  bitmap_layer_destroy(icon_layer);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  //Should update the screen saying you're changing direction.
  text_layer_set_text(layer_destination, "Switching Direction");
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context){
  direction = (direction ^ 1) & 0x1; // XOR by 1
  send_cmd();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Fetching data.");
  send_cmd();
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push(nb_selector_stop, true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push(nb_selector, true);
}

void pop_window(NumberWindow *number_window, void *context) {
  window_stack_pop(true);
  send_cmd();
}

void config_provider(Window *window) {
  // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, false);
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
      });

  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);

  nb_selector = window_create();
  window_set_background_color(nb_selector, GColorBlack);
  window_set_fullscreen(nb_selector, false);
  window_set_window_handlers(nb_selector, (WindowHandlers) {
      .load = window_nb_selector_load,
      .unload = window_nb_selector_unload
      });

  nb_selector_stop = window_create();
  window_set_background_color(nb_selector_stop, GColorBlack);
  window_set_fullscreen(nb_selector_stop, false);
  window_set_window_handlers(nb_selector_stop, (WindowHandlers) {
      .load = window_nb_selector_load,
      .unload = window_nb_selector_unload
      });


  route = 97;
  stop = 3050;
  window_set_user_data(nb_selector, (void*)&route);
  window_set_user_data(nb_selector_stop, (void*)&stop);

  wind_bus_sel = number_window_create("Bus #",  (NumberWindowCallbacks) {.selected = pop_window}, NULL);
  number_window_set_step_size(wind_bus_sel, 1);
  number_window_set_min(wind_bus_sel, 1);
  number_window_set_max(wind_bus_sel, 9999);
  number_window_set_value(wind_bus_sel, 96);

  wind_stop_sel = number_window_create("Stop #",  (NumberWindowCallbacks) {.selected = pop_window}, NULL);
  number_window_set_step_size(wind_stop_sel, 1);
  number_window_set_min(wind_stop_sel, 1);
  number_window_set_max(wind_stop_sel, 9999);
  number_window_set_value(wind_stop_sel, 3011);

  const bool animated = true;
  window_stack_push(window, animated);
  //window_stack_push((Window*)wind_bus_sel, true);
}

static void deinit(void) {
  window_destroy(window);
  number_window_destroy(wind_bus_sel);
  number_window_destroy(wind_stop_sel);
  window_destroy(nb_selector);
  window_destroy(nb_selector_stop);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}






