#include <pebble.h>

#define FRAME_DURATION 150
#define FRAME_COUNT 5

static Layer *s_window_layer;
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_bitmap_layer;

static AppTimer *s_animation_timer = NULL;
static int s_frame_index = 0;
static GBitmap *s_frames[FRAME_COUNT];

static const uint32_t FRAME_RESOURCE_IDS[FRAME_COUNT] = {
#ifdef PBL_COLOR
  RESOURCE_ID_IDLE_FRAME_0,
  RESOURCE_ID_IDLE_FRAME_1,
  RESOURCE_ID_IDLE_FRAME_2,
  RESOURCE_ID_IDLE_FRAME_3,
  RESOURCE_ID_IDLE_FRAME_4,
#else
  // 1-bit aplite versions
  RESOURCE_ID_HORNET_IDLE_BW_0,
  RESOURCE_ID_HORNET_IDLE_BW_1,
  RESOURCE_ID_HORNET_IDLE_BW_2,
  RESOURCE_ID_HORNET_IDLE_BW_3,
  RESOURCE_ID_HORNET_IDLE_BW_4,
#endif
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

static void prv_unobstructed_will_change(GRect final_unobstructed_screen_area,
                                         void *context) {
  // TODO hide hornet quote layer 
}

static void prv_unobstructed_did_change(void *context) {
  GRect full_bounds = layer_get_bounds(s_window_layer);
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  bool obstructed = !grect_equal(&full_bounds, &bounds);

  // Keep BT icon hidden when obstructed, otherwise restore based on connection
  if (obstructed) {
    //keep text hidden
  } else {
    //unhide quote text
  }
}


static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);

  int time_height = 40;
  int date_height = 15;
  int anim_height = 70;
  int padding = 2;  // tighter than before

  int total_height = time_height + date_height + anim_height + (padding * 2);
  int start_y = (bounds.size.h / 2) - (total_height / 2);

  int time_y = start_y;
  int date_y = time_y + time_height + padding;
  int anim_y = date_y + date_height + padding;

  GRect time_frame = layer_get_frame(text_layer_get_layer(s_time_layer));
  time_frame.origin.y = time_y;
  layer_set_frame(text_layer_get_layer(s_time_layer), time_frame);

  GRect date_frame = layer_get_frame(text_layer_get_layer(s_date_layer));
  date_frame.origin.y = date_y;
  layer_set_frame(text_layer_get_layer(s_date_layer), date_frame);

  GRect anim_frame = layer_get_frame(bitmap_layer_get_layer(s_bitmap_layer));
  anim_frame.origin.y = anim_y;
  layer_set_frame(bitmap_layer_get_layer(s_bitmap_layer), anim_frame);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  s_window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(s_window_layer);
  
  //Fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TRAJAN_BOLD_30));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TRAJAN_BOLD_16));
  
  //int16_t thirdHeight = bounds.size.h / 3;
  
  // Create the time TextLayer
  s_time_layer = text_layer_create(GRect(0, 10, bounds.size.w, 50));
//     s_time_layer = text_layer_create(
//       GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the date TextLayer
  
  s_date_layer = text_layer_create(GRect(0, 62, bounds.size.w, 30));
//   s_date_layer = text_layer_create(
//       GRect(0, PBL_IF_ROUND_ELSE(110, 104), bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Animation layer
  
  s_bitmap_layer = bitmap_layer_create(GRect(0, 94, bounds.size.w, 100));
  
  for(int i = 0; i < FRAME_COUNT; i ++){
    s_frames[i] = gbitmap_create_with_resource(FRAME_RESOURCE_IDS[i]);
  }
  
  s_animation_timer = app_timer_register(FRAME_DURATION, animation_timer_callback, NULL);
  

  // Add layers to the Window
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(s_window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(s_window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  bitmap_layer_set_alignment(s_bitmap_layer, GAlignCenter);
  
  //Quick peek API
   prv_unobstructed_change(0, NULL);
  prv_unobstructed_did_change(NULL);
  
  UnobstructedAreaHandlers handlers = {
  .will_change = prv_unobstructed_will_change,
  .change = prv_unobstructed_change,
  .did_change = prv_unobstructed_did_change
};
unobstructed_area_service_subscribe(handlers, NULL);

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
