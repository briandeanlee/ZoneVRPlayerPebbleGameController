#include <pebble.h>
#include "zvrpebble.h"
#include "pebbleassist.h"
#define DEBUGGING false

static Window *window;

static BitmapLayer *zvrlogo_bitmap_layer;

static GBitmap *zvrlogo;


static TextLayer *zvrtitle_text_layer;
static TextLayer *accel_text_layer;
static TextLayer *compass_text_layer;


static TextLayer *status_text_layer;

static AppTimer *accel_timer;
static AppTimer *reset_status_timer;
static char accel_text[256];
static char compass_text[32];

static void reset_status_timer_callback(void *data) {
  text_layer_set_text(status_text_layer, "Active");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
}

/*
static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *reward_tuple = dict_find(iter, KEY_REWARDS);
	Tuple *star_tuple = dict_find(iter, KEY_STARS);
	Tuple *balance_tuple = dict_find(iter, KEY_BALANCE);
	Tuple *status_tuple = dict_find(iter, KEY_STATUS);

	if (reward_tuple) {
		text_layer_set_text(reward_text_layer, reward_tuple->value->cstring);
	}
	if (star_tuple) {
		text_layer_set_text(star_text_layer, star_tuple->value->cstring);
	}
	if (balance_tuple) {
		text_layer_set_text(balance_text_layer, balance_tuple->value->cstring);
	}
	if (status_tuple) {
		text_layer_set_text(status_text_layer, status_tuple->value->cstring);
	}
}


static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped, %d", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}

*/
// -----------------------------------------------------------------------------------------------
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Select");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
}

// -----------------------------------------------------------------------------------------------
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Up");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
}

// -----------------------------------------------------------------------------------------------
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Down");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
}

// -----------------------------------------------------------------------------------------------
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void window_load(Window *window) {
	zvrtitle_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 26 } });
	text_layer_set_font(zvrtitle_text_layer, fonts_load_custom_font
   (resource_get_handle(RESOURCE_ID_CHALKDUSTER_24)));
	text_layer_set_overflow_mode(zvrtitle_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_background_color(zvrtitle_text_layer, GColorClear);
	text_layer_set_text_color(zvrtitle_text_layer, GColorBlack);
  text_layer_set_text_alignment(zvrtitle_text_layer, GTextAlignmentCenter );
	text_layer_set_text(zvrtitle_text_layer, "Z Control");
  
  accel_text_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { 144, 26 } });
	text_layer_set_font(accel_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_overflow_mode(accel_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_background_color(accel_text_layer, GColorClear);
	text_layer_set_text_color(accel_text_layer, GColorBlack);
  text_layer_set_text_alignment(accel_text_layer, GTextAlignmentCenter );
	text_layer_set_text(accel_text_layer, " ");

  compass_text_layer = text_layer_create((GRect) { .origin = { 87, 140 }, .size = { 70, 26 } });
	text_layer_set_font(compass_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_overflow_mode(compass_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_background_color(compass_text_layer, GColorClear);
	text_layer_set_text_color(compass_text_layer, GColorBlack);
  text_layer_set_text_alignment(compass_text_layer, GTextAlignmentCenter );
	text_layer_set_text(compass_text_layer, "0°");
  
  status_text_layer = text_layer_create((GRect) { .origin = { 0, 140 }, .size = { 70, 26 } });
	text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_overflow_mode(status_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_background_color(status_text_layer, GColorClear);
	text_layer_set_text_color(status_text_layer, GColorBlack);
  text_layer_set_text_alignment(status_text_layer, GTextAlignmentLeft );
	text_layer_set_text(status_text_layer, "Idle");
  
 
	zvrlogo_bitmap_layer = bitmap_layer_create((GRect) { .origin = { 0, 0 }, .size = { PEBBLE_WIDTH, PEBBLE_HEIGHT } });
	//bitmap_layer_set_compositing_mode(zvrlogo_bitmap_layer, GCompOpAssignInverted);
  bitmap_layer_set_alignment(zvrlogo_bitmap_layer, GAlignCenter);
	bitmap_layer_set_bitmap(zvrlogo_bitmap_layer, zvrlogo);

  layer_add_to_window(bitmap_layer_get_layer(zvrlogo_bitmap_layer), window);
	layer_add_to_window(text_layer_get_layer(zvrtitle_text_layer), window);
  layer_add_to_window(text_layer_get_layer(status_text_layer), window);
	layer_add_to_window(text_layer_get_layer(accel_text_layer), window);
	layer_add_to_window(text_layer_get_layer(compass_text_layer), window);


}

static void window_unload(Window *window) {
	text_layer_destroy(zvrtitle_text_layer);
  text_layer_destroy(status_text_layer);
  text_layer_destroy(accel_text_layer);
  text_layer_destroy(compass_text_layer);
	bitmap_layer_destroy(zvrlogo_bitmap_layer);
}


//static void app_message_init(void) {
//	app_message_open(128 /* inbound_size */, 2 /* outbound_size */);
//	app_message_register_inbox_received(in_received_handler);
//	app_message_register_inbox_dropped(in_dropped_handler);
//	app_message_register_outbox_sent(out_sent_handler);
//	app_message_register_outbox_failed(out_failed_handler);
//}

// Compass stuff
static void compass_handler(CompassHeadingData data) {

  // Determine status of the compass
  switch (data.compass_status) {
    // Compass data is not yet valid
    case CompassStatusDataInvalid:
      snprintf(compass_text, sizeof(compass_text), "Error..");
      text_layer_set_text(compass_text_layer, compass_text);
      break;

    // Compass is currently calibrating, but a heading is available
    case CompassStatusCalibrating:
      snprintf(compass_text, sizeof(compass_text), "~%d°", TRIGANGLE_TO_DEG((int)data.true_heading));
      text_layer_set_text(compass_text_layer, compass_text);
      layer_mark_dirty(text_layer_get_layer(compass_text_layer));


      break;
    // Compass data is ready for use, write the heading in to the buffer
    case CompassStatusCalibrated: 
      snprintf(compass_text, sizeof(compass_text), "%d°", TRIGANGLE_TO_DEG((int)data.true_heading));
      text_layer_set_text(compass_text_layer, compass_text);
      layer_mark_dirty(text_layer_get_layer(compass_text_layer));


      // Add app message output here
    
      break;

    // CompassStatus is unknown
    default:
      snprintf(compass_text, sizeof(compass_text), "Wait...");
      text_layer_set_text(compass_text_layer, compass_text);
      layer_mark_dirty(text_layer_get_layer(compass_text_layer));
      break;
  }
  

}

// Accelerometer stuff
// -----------------------------------------------------------------------------------------------
static void handle_accel_new_position(AccelData *accel) {
  
  if (DEBUGGING)
  {
  snprintf(accel_text, sizeof(accel_text), "%d %d %d", accel->x, accel->y, accel->z);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "accel position: %d %d %d", accel->x, accel->y, accel->z);
  text_layer_set_text(accel_text_layer, accel_text);
  layer_mark_dirty(text_layer_get_layer(accel_text_layer));
  }
  
  text_layer_set_text(status_text_layer, "Active");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));

  // Send Accelerometer data thru AppMessage
  
  /*
  // Update servo value
  int servo_pos = (accel->y + 1000) * 180/2000;
  if (servo_pos < 0) {
    servo_pos = 0;
  } else if (servo_pos > 180) {
    servo_pos = 180;
  }
  if (abs(servo_pos - g_last_servo_pos) > 0) {
    g_last_servo_pos = servo_pos;
    Tuplet value = TupletInteger(SERVO_KEY, servo_pos);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter != NULL) {
      dict_write_tuplet(iter, &value);
      dict_write_end(iter);
      app_message_outbox_send();
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Updated server position to: %d", servo_pos);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not write to outbox");
    }
  }
  */

}

// -----------------------------------------------------------------------------------------------
static void accel_timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };

  accel_service_peek(&accel);
  handle_accel_new_position(&accel);

  accel_timer = app_timer_register(1000 /* milliseconds */, accel_timer_callback, NULL);
}

static void handle_accel(AccelData *accel_data, uint32_t num_samples) {
  handle_accel_new_position(accel_data);
}

  
static void init(void) {
  // Turn on backlight
  light_enable(true);
	
  //zvrlogo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLEE);
	zvrlogo = gbitmap_create_with_resource(RESOURCE_ID_Z_LOGO);


	window = window_create();
  //IFDEF around this for API version + Watch type
	window_set_fullscreen(window, true);
  
	window_set_background_color(window, GColorBlack);
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	window_stack_push(window, true);

  accel_data_service_subscribe(0, handle_accel);
  accel_service_set_sampling_rate(1);
  accel_timer = app_timer_register(100 /* milliseconds */, accel_timer_callback, NULL);


  compass_service_subscribe(compass_handler);
  compass_service_set_heading_filter(1);


	//app_message_init();
}



static void deinit(void) {
  accel_data_service_unsubscribe();
  compass_service_unsubscribe();
	gbitmap_destroy(zvrlogo);
	window_destroy(window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Starting Z Controller - ZoneVR Game Controller for Pebble Watch");

	init();
	app_event_loop();
	deinit();
}