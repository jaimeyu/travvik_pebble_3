#include "pebble.h"
#include "nb_selector.h"
void window_nb_selector_load(Window* window);
void window_nb_selector_unload(Window* window);
static TextLayer *sel0;
void window_nb_selector_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  sel0= text_layer_create(GRect(0, 50 , 144, 68));
  text_layer_set_text_color(sel0, GColorWhite);
  text_layer_set_background_color(sel0, GColorClear);
  text_layer_set_font(sel0, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(sel0, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(sel0));

}

void window_nb_selector_unload(Window *window) {
  text_layer_destroy(sel0);
}


// End of nb selector



