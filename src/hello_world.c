#include <pebble.h>
#include <time.h>
#include <bitmap.h>

#define MAX_FLAKES 30
#define APPMSG_ACTION 1
#define DISABLE_SHAKE 2
#define DAY_OFFSET 3

Window *window;
TextLayer *text_layer;
Layer *bitmap_layer, *text_layer_bkgnd;
GBitmap *bkgnd, *bitmap;
AppTimer* phraseTimer;
static char txt[] = "sleeps: xxx__dd/mm/yy__battery: xxx%_conn: xxx     "; // Needs to be static because it's used by the system later.

int32_t disableShake;
int32_t dayOffset;

static struct TFlake{
    bool active;
    GPoint pos;
} flakes[MAX_FLAKES];

void reset_scene() {
    bmpFill(bitmap, GColorBlack);
    bmpCopy(bkgnd, bitmap);
    //
    BatteryChargeState state = battery_state_service_peek();
    GRect frame = GRect(68, 94, 8, 16);
    bmpFillRect(bitmap, frame, GColorBlack);
    if (!state.is_charging) {
        if (state.charge_percent >= 80) {
            // assume full charge
        } else if (state.charge_percent >= 60) {
            frame.size.h = 4;
            bmpFillRect(bitmap, frame, GColorPictonBlue);
        } else if (state.charge_percent >= 40) {
            frame.size.h = 8;
            bmpFillRect(bitmap, frame, GColorPictonBlue);
        } else if (state.charge_percent >= 20) {
            frame.size.h = 12;
            bmpFillRect(bitmap, frame, GColorPictonBlue);
        } else {
            bmpFillRect(bitmap, frame, GColorRed);
        }
    }
    //
    int c;
    for (c = 0; c < MAX_FLAKES; c++){
        flakes[c].active = 0;
        flakes[c].pos = GPoint(0, -1);
    }
}

static void update_display(Layer *layer, GContext *ctx) {
	graphics_draw_bitmap_in_rect(ctx, bitmap, GRect(0, 0, 144, 168));
    
    GRect bounds = layer_get_bounds(layer);
    GRect frame = GRect(0, 109, bounds.size.w, bounds.size.h);

    time_t def_time;
    struct tm *now;
    def_time = time(NULL);
	now = localtime(&def_time);    

    if (clock_is_24h_style())
        strftime(txt, sizeof(txt), "%H:%M", now);
    else
        strftime(txt, sizeof(txt), "%l:%M", now);

    frame.origin.x += 2;
    frame.origin.y += 2;
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, 
                       txt,
                       fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
                       frame,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL
                      );
    frame.origin.x -= 2;
    frame.origin.y -= 2;
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, 
                       txt,
                       fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
                       frame,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL
                      );
    //
    frame = GRect(65, 141, 16, 16);
    graphics_context_set_text_color(ctx, GColorWhite);
    snprintf(txt, sizeof(txt), "%d", now->tm_mday);
    graphics_draw_text(ctx, 
                       txt,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       frame,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL
                      );
}

int isEmpty(int x, int y){
    if (gcolor_equal(bmpGetPixel(bitmap, x * 2, y * 2), GColorBlack) &&
        gcolor_equal(bmpGetPixel(bitmap, x * 2 + 1, y * 2), GColorBlack) &&
        gcolor_equal(bmpGetPixel(bitmap, x * 2, y * 2 + 1), GColorBlack) &&
        gcolor_equal(bmpGetPixel(bitmap, x * 2 + 1, y * 2 + 1), GColorBlack))
        return 1;
    else
        return 0;
}

static void handle_phrase_timeout(void *data) {
    phraseTimer = NULL;

    layer_set_hidden(text_layer_bkgnd, true);
    layer_set_hidden(bitmap_layer, false);
}

static void easter_egg() {
    if (phraseTimer)
        return;
   
    strcpy(txt, "\nIt's\nHere\n\n:-)");
    
    text_layer_set_text(text_layer, txt);
    
    Layer *root_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(root_layer);
    layer_set_frame(text_layer_get_layer(text_layer), GRect(0, 0, frame.size.w, frame.size.h));
    
    layer_set_hidden(bitmap_layer, true);
    layer_set_hidden(text_layer_bkgnd, false);
    phraseTimer = app_timer_register(15000, &handle_phrase_timeout, NULL);  // 15 second delay before hide
    
    vibes_long_pulse();
}

static void handle_accel(AccelAxisType axis, int32_t direction) {
    if (disableShake) {
        return;
    }
    if (phraseTimer) {
        return;
    }

    static char txt2[] = "                                        ";    
    
    strcpy(txt, "");
    
    int day = 60 * 60 * 24;
    int diff;
    time_t iNow, iTarget;
    struct tm *now;
    struct tm target;
    iNow = time(NULL); // now as a struct
	now = localtime(&iNow); // now as an int
    memcpy(&target, now, sizeof(struct tm)); // target is struct of mignight magic
    target.tm_hour = 0;
    target.tm_min = 0;
    target.tm_sec = 0;
    iNow = mktime(&target); // iNow = midnight today
    target.tm_mday = 25;
    target.tm_mon = 11;
    iTarget = mktime(&target); // iTarget = midnight 25 Dec
    diff = ((iTarget - iNow) / day) + dayOffset; // sec to days with offset

    if (diff < 0) {
        diff += 365; // does not account for leap years
    }
    snprintf(txt, sizeof(txt), "Sleeps: %d\n\n", diff);

    // todo : JSconfiguration for date format
    strftime(txt2, sizeof(txt), "%d/%m/%y", now);
    strcat(txt, txt2);
    strcat(txt, "\n\n");
    
    strcat(txt, "Battery: ");
    BatteryChargeState state = battery_state_service_peek();
    if (state.is_charging)
        strcat(txt, "?");
    else {
        snprintf(txt2, sizeof(txt2), "%d%%", state.charge_percent);
        strcat(txt, txt2);
    }
    strcat(txt, "\n");

    strcat(txt, "Conn: ");
    if (bluetooth_connection_service_peek())
        strcat(txt, "Yes");
    else
        strcat(txt, "No");
    
    text_layer_set_text(text_layer, txt);
    
    Layer *root_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(root_layer);
    layer_set_frame(text_layer_get_layer(text_layer), GRect(0, 0, frame.size.w, frame.size.h));
    
    layer_set_hidden(bitmap_layer, true);
    layer_set_hidden(text_layer_bkgnd, false);
    phraseTimer = app_timer_register(2000, &handle_phrase_timeout, NULL);  // 6 second delay before hide
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    // easter egg at mignight
    if ((tick_time->tm_hour == 0) && (tick_time->tm_min == 0) && (tick_time->tm_sec < 10)){
        int day = 60 * 60 * 24;
        int diff;
        time_t iNow, iTarget;
        struct tm *now;
        struct tm target;
        iNow = time(NULL); // now as a struct
    	now = localtime(&iNow); // now as an int
        memcpy(&target, now, sizeof(struct tm)); // target is struct of mignight magic
        target.tm_hour = 0;
        target.tm_min = 0;
        target.tm_sec = 0;
        iNow = mktime(&target); // iNow = midnight today
        target.tm_mday = 25;
        target.tm_mon = 11;
        iTarget = mktime(&target); // iTarget = midnight 25 Dec
        diff = ((iTarget - iNow) / day) + dayOffset; // sec to days with offset
        if (diff == 0) {
            easter_egg();
            return;
        }
    }
    
    int c;
    for (c = 0; c < MAX_FLAKES; c++)
        if (flakes[c].active){
            if (flakes[c].pos.y != -1) {
                if (flakes[c].pos.y == 83)
                    flakes[c].pos.y = -1;
                else if (!isEmpty(flakes[c].pos.x, flakes[c].pos.y + 1)){
                    // blocked below
                    if ((flakes[c].pos.x > 0) && isEmpty(flakes[c].pos.x - 1, flakes[c].pos.y + 1)){
                        bmpFillRect(bitmap, GRect(flakes[c].pos.x * 2, flakes[c].pos.y * 2, 2, 2), GColorBlack);
                        flakes[c].pos.y++;
                        if ((flakes[c].pos.x < 71) && isEmpty(flakes[c].pos.x + 1, flakes[c].pos.y + 1) && (rand()%2 == 0))
                            flakes[c].pos.x++;
                        else
                            flakes[c].pos.x--;
                    }else if ((flakes[c].pos.x < 71) && isEmpty(flakes[c].pos.x + 1, flakes[c].pos.y + 1)){
                        bmpFillRect(bitmap, GRect(flakes[c].pos.x * 2, flakes[c].pos.y * 2, 2, 2), GColorBlack);
                        flakes[c].pos.y++;
                        flakes[c].pos.x++;
                    }else
                        flakes[c].pos.y = -1;
                } else {
                    bmpFillRect(bitmap, GRect(flakes[c].pos.x * 2, flakes[c].pos.y * 2, 2, 2), GColorBlack);
                    flakes[c].pos.y++;
                }
            }
            if (flakes[c].pos.y == -1)
                flakes[c].active = 0;
            else
                bmpFillRect(bitmap, GRect(flakes[c].pos.x * 2, flakes[c].pos.y * 2, 2, 2), GColorWhite);
        }

    if (rand() % 4 == 0)
        for (c = 0; c < MAX_FLAKES; c++)
            if (!flakes[c].active){
                int t = rand() % 72;
                flakes[c].pos.x = t;
                flakes[c].pos.y = 0;
                while (!isEmpty(t, flakes[c].pos.y + 1)){
                    t++;
                    if (t == 144)
                        t = 0;
                    if (t == flakes[c].pos.x){
                        // full
                        reset_scene();
                        // vibes_short_pulse();
                    }
                }
                flakes[c].pos.x = t;
                flakes[c].active = 1;
                break;
        }     
    
	layer_mark_dirty(bitmap_layer);
}

static void appmsg_in_received(DictionaryIterator *received, void *context) {
    Tuple *action_tuple = dict_find(received, APPMSG_ACTION);
    Tuple *disable_shake_tuple, *day_offset_tuple;
    int32_t v;
    if (action_tuple) {
        int32_t action = action_tuple->value->int32;
        switch (action) {
            case (0):
                // get settings
                // todo : somehow return the current settings
                break;
            case (1):
                // set settings
                disable_shake_tuple = dict_find(received, DISABLE_SHAKE);
                if (disable_shake_tuple) {
                    v = disable_shake_tuple->value->int32;
                    if (v != disableShake) {
                        disableShake = v;
                        persist_write_int(DISABLE_SHAKE, disableShake);
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "SET disableShake : %d", (int)disableShake);
                    }
                }
                day_offset_tuple = dict_find(received, DAY_OFFSET);
                if (day_offset_tuple) {
                    v = day_offset_tuple->value->int32;
                    if (v != dayOffset) {
                        dayOffset = v;
                        persist_write_int(DAY_OFFSET, dayOffset);
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "SET dayOffset : %d", (int)dayOffset);
                    }
                }
            break;
        }
    }
}

void handle_init(void) {
    // main scene
    bitmap = gbitmap_create_blank(GSize(144, 168), GBitmapFormat8Bit);

    // background from resouce
    bkgnd = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BKGND_2);
    
    // initialization
    phraseTimer = NULL;
    reset_scene();

	// Create a window
	window = window_create();

    // Create text background layer
    text_layer_bkgnd = layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), text_layer_bkgnd);	
    layer_set_hidden(text_layer_bkgnd, true);	

    // Create text layer
	text_layer = text_layer_create(GRect(0, 0, 144, 154));
	
	// Set the text, font, and text alignment
	text_layer_set_text(text_layer, "");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_background_color(text_layer, GColorBlack);
    text_layer_set_text_color(text_layer, GColorWhite);
	
	// Add the text layer to the window
	layer_add_child(text_layer_bkgnd, text_layer_get_layer(text_layer));

    // Create bitmap layer
    bitmap_layer = layer_create(GRect(0, 0, 144, 168));
    layer_set_update_proc(bitmap_layer, update_display);
    
    // Add bitmap layer
    layer_add_child(window_get_root_layer(window), bitmap_layer);
   
	// Push the window
	window_stack_push(window, true);

    // tick event
    tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);

    // settings
    disableShake = persist_exists(DISABLE_SHAKE) ? persist_read_int(DISABLE_SHAKE) : 0;
    dayOffset = persist_exists(DAY_OFFSET) ? persist_read_int(DAY_OFFSET) : 0;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "disableShake : %d", (int)disableShake);
    
    // accel event
    accel_tap_service_subscribe(&handle_accel);

    // config appSetting callback
    app_message_register_inbox_received(&appmsg_in_received);
    app_message_open(128, 128);

}

void handle_deinit(void) {
    // unsubscribes
    accel_tap_service_unsubscribe();
    tick_timer_service_unsubscribe();
    
	// Destroy the text layer
	text_layer_destroy(text_layer);
    layer_destroy(text_layer_bkgnd);

    // Destroy graphics
    layer_destroy(bitmap_layer);

    // background
    gbitmap_destroy(bkgnd);

    // main scene
    gbitmap_destroy(bitmap);
    
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}