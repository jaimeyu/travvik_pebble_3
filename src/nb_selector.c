#include "pebble.h"
#include "nb_selector.h"

// methods

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_up_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void nb_selector_config_provider(Window *window);

static TextLayer *sel0;
void window_nb_selector_load(Window *window) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Created nb selector.");
  Layer *window_layer = window_get_root_layer(window);

  sel0= text_layer_create(GRect(0, 50 , 144, 68));
  text_layer_set_text_color(sel0, GColorWhite);
  text_layer_set_background_color(sel0, GColorClear);
  text_layer_set_font(sel0, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(sel0, GTextAlignmentCenter);
  text_layer_set_text(sel0, "GPS tracking");
  layer_add_child(window_layer, text_layer_get_layer(sel0));


  window_set_click_config_provider(window, 
          nb_selector_config_provider);

}

void window_nb_selector_unload(Window *window) {
  text_layer_destroy(sel0);
}
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  text_layer_set_text(sel0, "Switching Direction");
}
static void select_up_click_handler(ClickRecognizerRef recognizer, void *context){
  text_layer_set_text(sel0, "Switching Direction");
}
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context){
  text_layer_set_text(sel0, "Switching Direction");
}

static void nb_selector_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_DOWN, select_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, select_up_click_handler);
}




// End of nb selector



