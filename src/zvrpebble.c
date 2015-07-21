#include <pebble.h>
#include "zvrpebble.h"
#include "pebbleassist.h"


static Window *window;

static BitmapLayer *zvrlogo_bitmap_layer;

static GBitmap *zvrlogo;


static TextLayer *zvrtitle_text_layer;


enum {
	KEY_REWARDS,
	KEY_STARS,
	KEY_BALANCE,
	KEY_STATUS,
};

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

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(zvrtitle_text_layer, "Updating...");
	//app_message_outbox_send();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
	zvrtitle_text_layer = text_layer_create((GRect) { .origin = { 16, 16 }, .size = { 78, 26 } });
	text_layer_set_font(zvrtitle_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_overflow_mode(zvrtitle_text_layer, GTextOverflowModeTrailingEllipsis);
	text_layer_set_background_color(zvrtitle_text_layer, GColorClear);
	text_layer_set_text_color(zvrtitle_text_layer, GColorBlack);
	text_layer_set_text(zvrtitle_text_layer, "ZoneVR\r\nController");
  /*
	reward_bitmap_layer = bitmap_layer_create((GRect) { .origin = { 44, 98 }, .size = { 30, 30 } });
	bitmap_layer_set_compositing_mode(reward_bitmap_layer, GCompOpAssignInverted);
	bitmap_layer_set_alignment(reward_bitmap_layer, GAlignCenter);
	bitmap_layer_set_bitmap(reward_bitmap_layer, reward);

	star_bitmap_layer = bitmap_layer_create((GRect) { .origin = { 44, 132 }, .size = { 30, 30 } });
	bitmap_layer_set_compositing_mode(star_bitmap_layer, GCompOpAssignInverted);
	bitmap_layer_set_alignment(star_bitmap_layer, GAlignCenter);
	bitmap_layer_set_bitmap(star_bitmap_layer, star);
  */
	zvrlogo_bitmap_layer = bitmap_layer_create((GRect) { .origin = { 0, 0 }, .size = { PEBBLE_WIDTH, PEBBLE_HEIGHT } });
	bitmap_layer_set_alignment(zvrlogo_bitmap_layer, GAlignCenter);
	bitmap_layer_set_bitmap(zvrlogo_bitmap_layer, zvrlogo);

  layer_add_to_window(bitmap_layer_get_layer(zvrlogo_bitmap_layer), window);
	layer_add_to_window(text_layer_get_layer(zvrtitle_text_layer), window);
}

static void window_unload(Window *window) {
	text_layer_destroy(zvrtitle_text_layer);
	bitmap_layer_destroy(zvrlogo_bitmap_layer);
}


//static void app_message_init(void) {
//	app_message_open(128 /* inbound_size */, 2 /* outbound_size */);
//	app_message_register_inbox_received(in_received_handler);
//	app_message_register_inbox_dropped(in_dropped_handler);
//	app_message_register_outbox_sent(out_sent_handler);
//	app_message_register_outbox_failed(out_failed_handler);
//}

  
static void init(void) {
	zvrlogo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLEE);

	window = window_create();
	window_set_fullscreen(window, true);
	window_set_background_color(window, GColorBlack);
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	window_stack_push(window, true);

	//app_message_init();
}

static void deinit(void) {
	gbitmap_destroy(zvrlogo);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}