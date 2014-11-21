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

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2,         // TUPLE_CSTRING
};

enum TRIP_KEYS {
  REQ_BUS_NB = 0,     // TUPLE_INT bus #
  REQ_STOP_NB = 1,      // TUPLE_INT stop #
  TRIP_ARRIVAL = 2,      // TUPLE_CSTRING stop name
  TRIP_DESTINATION = 3, // TUPLE_CSTRING
};




static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, //0
  RESOURCE_ID_IMAGE_CLOUD, //1
  RESOURCE_ID_IMAGE_RAIN, //2
  RESOURCE_ID_IMAGE_SNOW //3
};

static
int itoa(int value, char *sp, int radix);

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  static char nb[16];
  static char sb[16];
  static char ab[16];
  //itoa((int)new_tuple->value,nb,15);

  switch (key) {
    case REQ_BUS_NB:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: BUS_NB:%d", (int)new_tuple->value->int32);
      itoa((int)new_tuple->value->int32,nb,10);
      text_layer_set_text(temperature_layer, nb);
      break;
    case REQ_STOP_NB:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: stop_NB:%d", (int)new_tuple->value->int32);
      itoa((int)new_tuple->value->int32,sb,10);
      text_layer_set_text(city_layer, sb);
      break;
    case TRIP_ARRIVAL:

      itoa((int)new_tuple->value->int32,ab,10);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: arrival %d", (int)new_tuple->value->int32);
      //text_layer_set_text(arrival_layer, ab);
      break;
    case TRIP_DESTINATION:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "RCVD: dst:%s", new_tuple->value->cstring);
      text_layer_set_text(arrival_layer, new_tuple->value->cstring);
      break;
    default:
      break;
  }
  /*
     case WEATHER_ICON_KEY:
     if (icon_bitmap) {
     gbitmap_destroy(icon_bitmap);
     }
     icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
     bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
     break;

     case WEATHER_TEMPERATURE_KEY:
  // App Sync keeps new_tuple in sync_buffer, so we may use it directly
  text_layer_set_text(temperature_layer, new_tuple->value->cstring);
  break;

  case WEATHER_CITY_KEY:
  text_layer_set_text(city_layer, new_tuple->value->cstring);
  break;
  */
}

static void send_cmd(void) {
  /*
     Tuplet bus_values[] = {
     TupletInteger(REQ_BUS_NB, 96),
     TupletInteger(REQ_STOP_NB, 3011),
     TupletInteger(TRIP_ARRIVAL, -1),
     TupletCString(TRIP_DESTINATION, "                "),
     };
     */
  Tuplet value = TupletInteger(REQ_BUS_NB, 95);
  Tuplet stopnb = TupletInteger(REQ_STOP_NB, 3011);
  Tuplet arrival = TupletInteger(TRIP_ARRIVAL, -1);
  Tuplet dst = TupletCString(TRIP_DESTINATION, "Loading");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_tuplet(iter, &stopnb);
  dict_write_tuplet(iter, &arrival);
  dict_write_tuplet(iter, &dst);
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
    TupletInteger(REQ_BUS_NB, 96),
    TupletInteger(REQ_STOP_NB, 3011),
    TupletInteger(TRIP_ARRIVAL, -1),
    TupletCString(TRIP_DESTINATION, "                "),
  };
  /*
     Tuplet initial_values[] = {
     TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
     TupletCString(WEATHER_TEMPERATURE_KEY, "1234\u00B0C"),
     TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
     };
     */
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
void config_provider(Window *window) {
  // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);

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

  wind_bus_sel = number_window_create("Bus #",  (NumberWindowCallbacks) {.selected = NULL}, NULL);
  number_window_set_step_size(wind_bus_sel, 1);
  number_window_set_min(wind_bus_sel, 1);
  number_window_set_max(wind_bus_sel, 9999);
  number_window_set_value(wind_bus_sel, 96);

  wind_stop_sel = number_window_create("Stop #",  (NumberWindowCallbacks) {.selected = NULL}, NULL);
  number_window_set_step_size(wind_stop_sel, 1);
  number_window_set_min(wind_stop_sel, 1);
  number_window_set_max(wind_stop_sel, 9999);
  number_window_set_value(wind_stop_sel, 96);


  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
  //window_stack_push((Window*)wind_bus_sel, true);
}

static void deinit(void) {
  window_destroy(window);
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s:value:%d", __func__,value);
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
