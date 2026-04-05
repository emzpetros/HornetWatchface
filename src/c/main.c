#include <pebble.h>

#define FRAME_DURATION 150
#define FRAME_COUNT 5

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_bitmap_layer;

static AppTimer *s_animation_timer = NULL;
static int s_frame_index = 0;
static GBitmap *s_frames[FRAME_COUNT];

static const uint32_t FRAME_RESOURCE_IDS[FRAME_COUNT] ={
  RESOURCE_ID_IDLE_FRAME_0,
  RESOURCE_ID_IDLE_FRAME_1,
  RESOURCE_ID_IDLE_FRAME_2,
  RESOURCE_ID_IDLE_FRAME_3,
  RESOURCE_ID_IDLE_FRAME_4,
};

//
static AppTimer *s_quote_timer = NULL;

//Fonts
static GFont s_time_font;
static GFont s_date_font;

static void animation_timer_callback(void *context){
  s_frame_index = (s_frame_index + 1) % FRAME_COUNT; 
  bitmap_layer_set_bitmap(s_bitmap_layer, s_frames[s_frame_index]);
  layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
  
    s_animation_timer = app_timer_register(FRAME_DURATION, animation_timer_callback , NULL);
}

static void quote_timer_callback(void *context){
  
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                                                    "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);

  // Write the current date into a buffer
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tick_time);

  // Display the date
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TRAJAN_52));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TRAJAN_24));
  
  //int16_t thirdHeight = bounds.size.h / 3;
  
  // Create the time TextLayer
  s_time_layer = text_layer_create(
      GRect(0, 20, bounds.size.w, 50));
//     s_time_layer = text_layer_create(
//       GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the date TextLayer
  
  s_date_layer = text_layer_create(
      GRect(0, 20 +PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 30));
//   s_date_layer = text_layer_create(
//       GRect(0, PBL_IF_ROUND_ELSE(110, 104), bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Animation layer
  
  s_bitmap_layer = bitmap_layer_create(GRect(0, 100, bounds.size.w, 100));
  
  for(int i = 0; i < FRAME_COUNT; i ++){
    s_frames[i] = gbitmap_create_with_resource(FRAME_RESOURCE_IDS[i]);
  }
  
  s_animation_timer = app_timer_register(FRAME_DURATION, animation_timer_callback, NULL);
  

  // Add layers to the Window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  bitmap_layer_set_alignment(s_bitmap_layer, GAlignCenter);
}

static void main_window_unload(Window *window) {
  //Fonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  
  if(s_animation_timer){
    app_timer_cancel(s_animation_timer);
    s_animation_timer = NULL;
  }
  
   for (int i = 0; i < FRAME_COUNT; i++) {
    gbitmap_destroy(s_frames[i]);
    s_frames[i] = NULL;
  }
  
  
  bitmap_layer_destroy(s_bitmap_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
