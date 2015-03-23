#include "pebble.h"
#include "nb_selector.h"
#include "travvik.h"
// methods

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_up_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void nb_selector_config_provider(Window *window);
char* digit2str(int digit);

static TextLayer *number[4];
static int inb[4];
static int *final_number;
static TextLayer *underscore[4];
static uint8_t curCount = 0;

extern int stop;

void setNumberDefault(TextLayer *curLayer){
    text_layer_set_text_color(curLayer, GColorWhite);
    text_layer_set_background_color(curLayer, GColorBlack);
    text_layer_set_font(curLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(curLayer, GTextAlignmentLeft);
}

void window_nb_selector_load(Window *window) {
    int i = 0;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Created nb selector.");
    Layer *window_layer = window_get_root_layer(window);

    final_number = (int*) window_get_user_data(window);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "user data:%d", *final_number);
    for (i = 0; i < 4; i++) {
        inb[i] = 0;

        underscore[i] = text_layer_create(GRect(0 + (i*40), 65 , 30, 40));
        setNumberDefault(underscore[i]);
        text_layer_set_text(underscore[i], "");
        layer_add_child(window_layer, text_layer_get_layer(underscore[i]));

        number[i] = text_layer_create(GRect(0 + (i*40), 50 , 50, 40));
        setNumberDefault(number[i]);
        text_layer_set_text(number[i], "0");
        layer_add_child(window_layer, text_layer_get_layer(number[i]));

    }
    text_layer_set_text(underscore[0], "_");

    int c = *final_number / 1000 % 10;
    text_layer_set_text(number[0], digit2str(c));
    
    c = *final_number / 100 % 10;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "user data:%d", c);
    text_layer_set_text(number[1], digit2str(c));
    
    c = *final_number / 10 % 10;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "user data:%d", c);
    text_layer_set_text(number[2], digit2str(c));

    c = *final_number % 10;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "user data:%d", c);
    text_layer_set_text(number[3], digit2str(c));




    window_set_click_config_provider(window, 
            (ClickConfigProvider)nb_selector_config_provider);

    curCount = 0;
}

void window_nb_selector_unload(Window *window) {
}


static void select_single_click_handler(ClickRecognizerRef recognizer, void *context){
    int count;
    curCount = (curCount + 1);

    count = curCount - 1;

    if (curCount > 3){

        //convert number selector to number
        *final_number = 
            (inb[0] * 1000) +
            (inb[1] * 100) +
            (inb[2] * 10) + 
            inb[3];
        /*
        APP_LOG(APP_LOG_LEVEL_DEBUG, "inb:%d", *final_number);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "inb0:%d", inb[0]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "inb1:%d",  inb[1]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "inb2:%d",  inb[2]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "inb3:%d",  inb[3]);
        */
        send_cmd();
        window_stack_pop(true);
    }
    else{
        text_layer_set_text(underscore[count],"");
        text_layer_set_text(underscore[curCount],"_");
    }

}

static char* STR_0 = "0";
static char* STR_1 = "1";
static char* STR_2 = "2";
static char* STR_3 = "3";
static char* STR_4 = "4";
static char* STR_5 = "5";
static char* STR_6 = "6";
static char* STR_7 = "7";
static char* STR_8 = "8";
static char* STR_9 = "9";

char* digit2str(int digit){
    char *result = STR_0;
    switch(digit){
        default:
        case 0:
            result = STR_0;
            break;
        case 1:
            result = STR_1;
            break;
        case 2:
            result = STR_2; 
            break;
        case 3: 
            result = STR_3;
            break;
        case 4: 
            result = STR_4;
            break;
        case 5: 
            result = STR_5;
            break;
        case 6: 
            result = STR_6;
            break;
        case 7: 
            result = STR_7;
            break;
        case 8: 
            result = STR_8;
            break;
        case 9: 
            result = STR_9;
            break;
    }
    return result;
}


static void select_up_click_handler(ClickRecognizerRef recognizer, void *context){
    inb[curCount] = (inb[curCount] + 1) % 10;
    text_layer_set_text(number[curCount], digit2str(inb[curCount]));
}
static void select_down_click_handler(ClickRecognizerRef recognizer, void *context){
    inb[curCount] = (inb[curCount] - 1);
    if (inb[curCount] < 0){
        inb[curCount] = 9;
    }
    text_layer_set_text(number[curCount], digit2str(inb[curCount]));

}

static void nb_selector_config_provider(Window *window) {
    window_single_click_subscribe(BUTTON_ID_DOWN, select_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, select_up_click_handler);
}




// End of nb selector



