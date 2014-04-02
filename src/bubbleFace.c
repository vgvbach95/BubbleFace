#include <pebble.h>

Window *window;
static Layer *layer;
static BitmapLayer *image_layer;

static TextLayer *time_layer;
static TextLayer *date_layer;
static GBitmap *b_image;

static GBitmap *b_connected;
static GBitmap *b_disconnected;
static BitmapLayer *blue_layer;

static GBitmap *above_0;
static GBitmap *above_25;
static GBitmap *above_50;
static GBitmap *above_75;
static GBitmap *charging;
static BitmapLayer *bat_layer;

static char date_text[6];
static char timeText[] = "00:00";

static void handle_battery(BatteryChargeState charge_state)
{
  if(charge_state.is_charging)
    bitmap_layer_set_bitmap(bat_layer, charging);
  else
  {
    uint8_t charge = charge_state.charge_percent;
    if(charge >= 75)
      bitmap_layer_set_bitmap(bat_layer, above_75);
    else if(charge >= 50)
      bitmap_layer_set_bitmap(bat_layer, above_50);
    else if(charge >= 25)
      bitmap_layer_set_bitmap(bat_layer, above_25);
    else
      bitmap_layer_set_bitmap(bat_layer, above_0);
  }
}

static void handle_bluetooth(bool connected)
{
  if(connected)
    bitmap_layer_set_bitmap(blue_layer, b_connected);
  else
    bitmap_layer_set_bitmap(blue_layer, b_disconnected);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  
  time_t now = time(NULL);
  struct tm * currentTime = localtime(&now);
  
  strftime(date_text, sizeof(date_text), "%m/%d", currentTime);
  text_layer_set_text(date_layer, date_text);
  
  char *time_format;
  if(clock_is_24h_style())
    time_format = "%R";
  else
    time_format = "%I:%M";
  
  strftime(timeText, sizeof(timeText), time_format, currentTime);
  
  if(!clock_is_24h_style() && (timeText[0] == '0'))
    memmove(timeText, &timeText[1], sizeof(timeText)-1);
  
  text_layer_set_text(time_layer, timeText);
  
}

static void handle_init(void) {
  //create window
	window = window_create();
  window_stack_push(window, true);
  
  //set background image
  b_image = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
  //bluetooth images
	b_connected = gbitmap_create_with_resource(RESOURCE_ID_B_CON);
  b_disconnected = gbitmap_create_with_resource(RESOURCE_ID_B_DCON);
  //battery images
  above_75 = gbitmap_create_with_resource(RESOURCE_ID_ABOVE_75);
  above_50 = gbitmap_create_with_resource(RESOURCE_ID_ABOVE_50);
  above_25 = gbitmap_create_with_resource(RESOURCE_ID_ABOVE_25);
  above_0 = gbitmap_create_with_resource(RESOURCE_ID_ABOVE_ZERO);
  charging = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
  
  //initialize the layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  
  //set up image Background --MUST HAPPEN FIRST--
  image_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(image_layer, b_image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  
  //Set Font
  GFont font = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);

  //date
  date_layer = text_layer_create(GRect(60, 75, bounds.size.w-50, bounds.size.h - 20));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_font(date_layer, font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  
  //time Layer set up
  time_layer = text_layer_create(GRect(60, 50, bounds.size.w-71, bounds.size.h - 20));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text_alignment(time_layer, GTextAlignmentRight);
  text_layer_set_font(time_layer, font);
  
  handle_minute_tick(NULL, 0);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  //battery layer
  bat_layer = bitmap_layer_create(GRect(3,138, 25, 25));
  handle_battery(battery_state_service_peek());
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bat_layer));

  //bluetooth layer
  blue_layer = bitmap_layer_create(GRect(125,2, 15,15));
  handle_bluetooth(bluetooth_connection_service_peek());
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(blue_layer));
  
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  battery_state_service_subscribe(&handle_battery);
  
}

static void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  gbitmap_destroy(b_image);
  bitmap_layer_destroy(image_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  bitmap_layer_destroy(bat_layer);
  bitmap_layer_destroy(blue_layer);
  layer_destroy(layer);
  //FINAL destroy window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}