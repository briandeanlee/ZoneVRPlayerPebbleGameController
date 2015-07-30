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
int old_compass_value;

enum OutMessageKey {
  OUT_RESERVED_KEY = 0x0,                  // TUPLE_INT
  BUTTON_PRESSED_KEY = 0x1,            // TUPLE_CHAR
  COMPASS_DATA_KEY   = 0x2,            // TUPLE_INT
  ACCELEROMETER_DATA_KEY = 0x3,
};

enum InMessageKey {
  IN_RESERVED_KEY = 0x0,                  // TUPLE_INT
  GAME_STATE_CHANGE_KEY = 0x1,            // TUPLE_CHAR
  APP_CONTROL_KEY   = 0x2,            // TUPLE_INT
  DEVICE_CONTROL_KEY = 0X3,           // TUPLE_CHAR
};

static void reset_status_timer_callback(void *data) {
  text_layer_set_text(status_text_layer, "Active");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
}


static bool send_char_data_to_phone(int command, char *data) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "send_char_data_to_phone IN");


  //if (command == 0 || data == NULL) return false;
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "char null iter");
    return false;
  }
  
  Tuplet tuple = TupletCString(command, data);
  dict_write_tuplet(iter, &tuple);
  dict_write_end(iter);
  app_message_outbox_send();
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "send_char_data_to_phone OUT");


  return true;
}

static bool send_int_data_to_phone(int command, int data) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "int null iter");
    return false;
  }

  Tuplet tuple = TupletInteger(command, data);
  dict_write_tuplet(iter, &tuple);
  dict_write_end(iter);
  app_message_outbox_send();
  return true;
}

// -----------------------------------------------------------------------------------------------
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "select_click_handler IN");


  //if (reset_status_timer!=NULL) app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Pause");
  send_char_data_to_phone(BUTTON_PRESSED_KEY, "Select");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "select_click_handler OUT");


}

// -----------------------------------------------------------------------------------------------
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "up_click_handler IN");


  //if (reset_status_timer!=NULL) app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Up");
  send_char_data_to_phone(BUTTON_PRESSED_KEY, "Up");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "up_click_handler OUT");


}

// -----------------------------------------------------------------------------------------------
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
       // APP_LOG(APP_LOG_LEVEL_DEBUG, "down_click_handler IN");


  //if (reset_status_timer!=NULL) app_timer_cancel(reset_status_timer);
  text_layer_set_text(status_text_layer, "Down");
  send_char_data_to_phone(BUTTON_PRESSED_KEY, "Down");
  layer_mark_dirty(text_layer_get_layer(status_text_layer));
  reset_status_timer = app_timer_register(500 /* milliseconds */, reset_status_timer_callback, NULL);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "down_click_handler OUT");


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

  compass_text_layer = text_layer_create((GRect) { .origin = { 86, 140 }, .size = { 70, 26 } });
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

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}


void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox message sent!");
}
void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox message failed!: %s", translate_error(reason));
}
void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox message dropped!: %s", translate_error(reason));
}
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Inbox message received!");
  Tuple *gameState_tuple = dict_find(iterator, GAME_STATE_CHANGE_KEY);
  Tuple *deviceControl_tuple = dict_find(iterator, DEVICE_CONTROL_KEY);

  if (gameState_tuple)
  {
      APP_LOG(APP_LOG_LEVEL_INFO, "gameState message received: %s", gameState_tuple->value->cstring );
      text_layer_set_text(accel_text_layer, gameState_tuple->value->cstring);
      layer_mark_dirty(text_layer_get_layer(accel_text_layer));
  }
  else if (deviceControl_tuple)
  {
      APP_LOG(APP_LOG_LEVEL_INFO, "deviceControl message received: %s",deviceControl_tuple->value->cstring );

      if (strcmp(deviceControl_tuple->value->cstring,"BACKLIGHT_ON") ==0)
        light_enable(true);
      if (strcmp(deviceControl_tuple->value->cstring,"BACKLIGHT_OFF") ==0)
        light_enable(false);
      if (strcmp(deviceControl_tuple->value->cstring,"VIBRATE_SHORT") ==0)
      {
        vibes_cancel();
        vibes_short_pulse();
      }
      if (strcmp(deviceControl_tuple->value->cstring,"VIBRATE_LONG") ==0)
      {
        vibes_cancel();
        vibes_long_pulse();
      }
      if (strcmp(deviceControl_tuple->value->cstring,"VIBRATE_DOUBLE") ==0)
      {
        vibes_cancel();
        vibes_double_pulse();
      }


  }
}


static void app_message_init(void) {
	//app_message_open(256 /* inbound_size */,  /* outbound_size */);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
}

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
      if (old_compass_value != TRIGANGLE_TO_DEG((int)data.true_heading)) send_int_data_to_phone(COMPASS_DATA_KEY,TRIGANGLE_TO_DEG((int)data.true_heading));
      old_compass_value = TRIGANGLE_TO_DEG((int)data.true_heading);
      layer_mark_dirty(text_layer_get_layer(compass_text_layer));


      break;
    // Compass data is ready for use, write the heading in to the buffer
    case CompassStatusCalibrated: 
      snprintf(compass_text, sizeof(compass_text), "%d°", TRIGANGLE_TO_DEG((int)data.true_heading));
      text_layer_set_text(compass_text_layer, compass_text);
      if (old_compass_value != TRIGANGLE_TO_DEG((int)data.true_heading)) send_int_data_to_phone(COMPASS_DATA_KEY,TRIGANGLE_TO_DEG((int)data.true_heading));
      old_compass_value = TRIGANGLE_TO_DEG((int)data.true_heading);
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
  reset_status_timer = NULL;
	app_message_init();
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

  old_compass_value=0;
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