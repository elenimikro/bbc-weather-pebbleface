#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_IMAGE 2
#define SUNNY 3
#define CLOUDY 4
#define LIGHT_CLOUD 5
#define SUNNY_INTERVALS 6
#define IMAGE_CLEAR 7
#define IMAGE_PARTLY_CLOUDY 8
#define IMAGE_SHOWERS 9
#define LOCATION_NAME 20
#define STORAGE_VERSION_KEY 124 // any previously unused value
#define CURRENT_STORAGE_VERSION 1.2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_weather_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_temperature_text_layer;
static TextLayer *s_weather_text_layer;
// static TextLayer *s_loading_text_layer;


static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    text_layer_set_text(s_weather_text_layer, "Connected. Wait for it...");
  } else {
    text_layer_set_text(s_weather_text_layer, "Disconnected. Check Bluetooth Connection...");
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char date_buffer[] = "Mon 11 Jan";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  strftime(date_buffer, sizeof("Mon 11 Jan"), "%a %e %b", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void set_weather_icon(int resource_id){
  s_background_bitmap = gbitmap_create_with_resource(resource_id);
  bitmap_layer_set_bitmap(s_weather_layer, s_background_bitmap);
}

static void update_weather_image(int weather_image){
   if (weather_image == SUNNY){
        set_weather_icon(RESOURCE_ID_IMAGE_WEATHER_SUNNY);
      }
      else if(weather_image == LIGHT_CLOUD){
        set_weather_icon(RESOURCE_ID_IMAGE_WEATHER_LIGHT_CLOUD);
      }
      else if( weather_image == CLOUDY){
        set_weather_icon(RESOURCE_ID_IMAGE_CLOUDY);
      }
      else if(weather_image == SUNNY_INTERVALS){
        set_weather_icon(RESOURCE_ID_IMAGE_SUNNY_INTERVALS);
      }
      else if(weather_image == IMAGE_CLEAR){
        set_weather_icon(RESOURCE_ID_IMAGE_CLEAR);
      }
      else if(weather_image == IMAGE_PARTLY_CLOUDY){
        set_weather_icon(RESOURCE_ID_IMAGE_PARTLY_CLOUDY);
      }
      else if(weather_image == IMAGE_SHOWERS){
        set_weather_icon(RESOURCE_ID_IMAGE_SHOWERS);
      }
      else{
        APP_LOG(APP_LOG_LEVEL_ERROR, "Weather Key %d not recognized!", (int)weather_image);
      }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 60 minutes
  if(tick_time->tm_min % 60 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
  
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[32];
  static char conditions_buffer[32];
  static char location_buffer[32];
  static char weather_layer_buffer[32];
  static int weather_image;
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", t->value->cstring);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      //update_weather_image();
      break;
    case LOCATION_NAME:
      snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
    case KEY_IMAGE:
      weather_image = (int)t->value->int32;
      update_weather_image(weather_image);
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", conditions_buffer, location_buffer);
  text_layer_set_text(s_temperature_text_layer, temperature_buffer);
  text_layer_set_text(s_weather_text_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 0, 144, 43));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
   // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 43, 144, 19));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Mon 1 Jan");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  
  // Create weather image Layer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BBC);
  s_weather_layer = bitmap_layer_create(GRect(0, 63, 75, 60));
  bitmap_layer_set_bitmap(s_weather_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_layer));
  
  //Create temperature text layer
  s_temperature_text_layer = text_layer_create(GRect(76, 70, 69, 60));
  text_layer_set_background_color(s_temperature_text_layer, GColorClear);
  text_layer_set_text_color(s_temperature_text_layer, GColorBlack);
  text_layer_set_text_alignment(s_temperature_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_temperature_text_layer, " ");
  
  
  //Create weather text Layer
  s_weather_text_layer = text_layer_create(GRect(0, 115, 144, 80));
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_text_color(s_weather_text_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentCenter);
  
  // Show current connection state
  bt_handler(connection_service_peek_pebble_app_connection());
  
  // Add it as a child layer to the Window's root layer
  text_layer_set_font(s_temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_text_layer));
  text_layer_set_font(s_weather_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_text_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_date_layer);
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_weather_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_temperature_text_layer);
  text_layer_destroy(s_weather_text_layer);
}



static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
   // Subscribe to Bluetooth updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bt_handler
  });

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}