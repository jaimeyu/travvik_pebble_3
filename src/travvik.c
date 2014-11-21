#include "pebble.h"

static Window *window;
static NumberWindow *wind_bus_sel;
static NumberWindow *wind_stop_sel;

static TextLayer *temperature_layer;
static TextLayer *city_layer;
static TextLayer *arrival_layer;
//static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
static uint8_t sync_buffer[128];

static volatile uint8_t direction = 0;
static volatile uint32_t route = 96;
static volatile uint32_t stop = 3011;
static volatile uint8_t busy = 0;

enum TRIP_KEYS {
  KEY_ROUTE = 0,     // TUPLE_INT bus #
  KEY_STOP_NUM = 1,      // TUPLE_INT stop #
  KEY_ETA = 2,      // TUPLE_CSTRING stop name
  KEY_DST = 3, // TUPLE_CSTRING
  KEY_STATION_STR = 4,
  KEY_DIRECTION = 5,
};

static int 
itoa(int value, char *sp, int radix);

static void 
sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void 
sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  static char nb[16];
  static char sb[16];
  static char ab[16];
  //itoa((int)new_tuple->value,nb,15);

  switch (key) {
    case KEY_ROUTE:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: BUS_NB:%d", (int)new_tuple->value->int32);
      itoa((int)new_tuple->value->int32,nb,10);
      text_layer_set_text(temperature_layer, nb);
      break;
    case KEY_STOP_NUM:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: stop_NB:%d", (int)new_tuple->value->int32);
      itoa((int)new_tuple->value->int32,sb,10);
      text_layer_set_text(city_layer, sb);
      break;
    case KEY_ETA:

      itoa((int)new_tuple->value->int32,ab,10);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: arrival %d", (int)new_tuple->value->int32);
      //text_layer_set_text(arrival_layer, ab);
      break;
    case KEY_DST:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: dst:");
      text_layer_set_text(arrival_layer, new_tuple->value->cstring);
      // make it unbusy.
      busy = 0;
      break;
    case KEY_DIRECTION:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: direction:");
//      text_layer_set_text(arrival_layer, new_tuple->value->cstring);
      // make it unbusy.
      busy = 0;
      break;
 
    case KEY_STATION_STR:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: station name:");
//      text_layer_set_text(arrival_layer, new_tuple->value->cstring);
      // make it unbusy.
      busy = 0;
      break;
 
    default:
      break;
  }
}

static void send_cmd(void) {
  // Don't send if we're busy sending a request.
  if (busy != 0){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Still handling old req");
    return;
  }

  //busy = 1;

  route = number_window_get_value(wind_bus_sel);
  stop = number_window_get_value(wind_stop_sel);

  Tuplet value =    TupletInteger(KEY_ROUTE, route);
  Tuplet stopnb =   TupletInteger(KEY_STOP_NUM, stop);
  Tuplet arrival =  TupletInteger(KEY_ETA, -1);
  Tuplet dst =      TupletCString(KEY_DST, "Loading");
  Tuplet station_str = TupletCString(KEY_DST, "");
  Tuplet dir = TupletInteger(KEY_DIRECTION, direction); 

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
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  //icon_layer = bitmap_layer_create(GRect(32, 10, 80, 80));
  //layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));

  temperature_layer = text_layer_create(GRect(0, 95, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  city_layer = text_layer_create(GRect(0, 125, 144, 68));
  text_layer_set_text_color(city_layer, GColorWhite);
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_font(city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(city_layer));

  arrival_layer = text_layer_create(GRect(0, 25, 144, 68));
  text_layer_set_text_color(arrival_layer, GColorWhite);
  text_layer_set_background_color(arrival_layer, GColorClear);
  text_layer_set_font(arrival_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(arrival_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(arrival_layer));



  Tuplet bus_values[] = {
    TupletInteger(KEY_ROUTE, 96),
    TupletInteger(KEY_STOP_NUM, 3011),
    TupletInteger(KEY_ETA, -1),
    TupletCString(KEY_DST, "                 "),
    TupletCString(KEY_STATION_STR, "         "),
    TupletInteger(KEY_DIRECTION, 0),
  };

    int inbound_size = app_message_inbox_size_maximum();
    int outbound_size = app_message_outbox_size_maximum();
    app_message_open(inbound_size, outbound_size);
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), bus_values, ARRAY_LENGTH(bus_values),
    sync_tuple_changed_callback, sync_error_callback, NULL);

  //send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }

  text_layer_destroy(city_layer);
  text_layer_destroy(arrival_layer);
  text_layer_destroy(temperature_layer);
  //  bitmap_layer_destroy(icon_layer);
}

void select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  //Should update the screen saying you're changing direction.
}

void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context){
  direction = (direction ^ 1) & 0x1; // XOR by 1
  send_cmd();
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Fetching data.");
  send_cmd();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push((Window*)wind_bus_sel, true);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push((Window*)wind_stop_sel, true);
}

void pop_window(NumberWindow *number_window, void *context) {
  window_stack_pop(true);
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
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
      });

  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);

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
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}



// Thanks to https://stackoverflow.com/questions/3440726/what-is-the-proper-way-of-implementing-a-good-itoa-function
// So I don't have to roll my own.
static int itoa(int value, char *sp, int radix)
{
  char tmp[16];// be careful with the length of the buffer
  char *tp = tmp;
  int i;
  unsigned v;

//  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s:value:%d", __func__,value);
  int sign = (radix == 10 && value < 0);
  if (sign)
    v = -value;
  else
    v = (unsigned)value;

  while (v || tp == tmp)
  {
    i = v % radix;
    v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
    if (i < 10)
      *tp++ = i+'0';
    else
      *tp++ = i + 'a' - 10;
  }

  int len = tp - tmp;

  if (sign)
  {
    *sp++ = '-';
    len++;
  }

  while (tp > tmp)
    *sp++ = *--tp;

  return len;
}
