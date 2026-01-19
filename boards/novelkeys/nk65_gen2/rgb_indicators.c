/*
 * Copyright (c) 2026 Yiancar
 *
 * SPDX-License-Identifier: MIT
 */

#include <zmk/event_manager.h>
#include <zmk/events/rgb_underglow_tick.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/rgb_underglow.h>
#include <zmk/keymap.h>

bool is_capslock_on;
bool is_layer1_on;
uint8_t current_brightness;
bool underglow_brightness_known;

static int hid_indicator_change_listener(const zmk_event_t *eh);

ZMK_LISTENER(hid_indicator, hid_indicator_change_listener);
ZMK_SUBSCRIPTION(hid_indicator, zmk_hid_indicators_changed);
ZMK_SUBSCRIPTION(hid_indicator, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(hid_indicator, zmk_rgb_underglow_tick_event);

static int hid_indicator_change_listener(const zmk_event_t *eh){
    const struct zmk_hid_indicators_changed *hid_ev = as_zmk_hid_indicators_changed(eh);
    struct zmk_rgb_underglow_tick_event *underglow_ev = as_zmk_rgb_underglow_tick_event(eh);

    if ((hid_ev = as_zmk_hid_indicators_changed(eh)) != NULL) {
        // Ignore indicator events until we have a valid brightness sample from underglow.
        if (!underglow_brightness_known) {
            return ZMK_EV_EVENT_BUBBLE;
        }
        if (hid_ev->indicators & (1 << 1)) {
            is_capslock_on = true;
        } else {
            is_capslock_on = false;
        }
        if (is_layer1_on || is_capslock_on) {
            if (current_brightness == 0) {
                zmk_rgb_underglow_on();
            }
        } else {
            if (current_brightness == 0) {
                zmk_rgb_underglow_off();
            }
        }
    } else if (as_zmk_layer_state_changed(eh) != NULL) {
        if (!underglow_brightness_known) {
            return ZMK_EV_EVENT_BUBBLE;
        }
        if (zmk_keymap_layer_active(1)) {
            is_layer1_on = true;
        } else {
            is_layer1_on = false;
        }
        if (is_layer1_on || is_capslock_on) {
            if (current_brightness == 0) {
                zmk_rgb_underglow_on();
            }
        } else {
            if (current_brightness == 0) {
                zmk_rgb_underglow_off();
            }
        }
    } else if ((underglow_ev = as_zmk_rgb_underglow_tick_event(eh)) != NULL) {
        underglow_brightness_known = true;
        current_brightness = underglow_ev->state.color.b;
        
        if (is_capslock_on) {
            underglow_ev->pixels[66] = (struct led_rgb){r : 150, g : 115, b : 8};
        } else {
            underglow_ev->pixels[66] = (struct led_rgb){r : 0, g : 0, b : 0};
        }
        if (zmk_keymap_layer_active(1)) {
            underglow_ev->pixels[67] = (struct led_rgb){r : 51, g : 71, b : 23};
        } else {
            underglow_ev->pixels[67] = (struct led_rgb){r : 0, g : 0, b : 0};
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}
