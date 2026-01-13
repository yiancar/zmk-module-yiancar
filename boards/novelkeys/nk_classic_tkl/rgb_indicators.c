/*
 * Copyright (c) 2025 Yiancar
 *
 * SPDX-License-Identifier: MIT
 */

#include <zmk/event_manager.h>
#include <zmk/events/rgb_underglow_tick.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/rgb_underglow.h>

bool is_capslock_on;
uint8_t current_brightness;

static int hid_indicator_change_listener(const zmk_event_t *eh);

ZMK_LISTENER(hid_indicator, hid_indicator_change_listener);
ZMK_SUBSCRIPTION(hid_indicator, zmk_hid_indicators_changed);
ZMK_SUBSCRIPTION(hid_indicator, zmk_rgb_underglow_tick_event);

static int hid_indicator_change_listener(const zmk_event_t *eh){
    const struct zmk_hid_indicators_changed *hid_ev = as_zmk_hid_indicators_changed(eh);
    struct zmk_rgb_underglow_tick_event *underglow_ev = as_zmk_rgb_underglow_tick_event(eh);

    if ((hid_ev = as_zmk_hid_indicators_changed(eh)) != NULL) {
        if (hid_ev->indicators & (1 << 1)){
            is_capslock_on = true;
            if (current_brightness == 0) {
                zmk_rgb_underglow_on();
            }
        } else {
            is_capslock_on = false;
            if (current_brightness == 0) {
                zmk_rgb_underglow_off();
            }
        }
    } else if ((underglow_ev = as_zmk_rgb_underglow_tick_event(eh)) != NULL) {
        current_brightness = underglow_ev->state.color.b;
        if (is_capslock_on) {
            for (uint8_t i = 50; i <= 55; i++) {
                underglow_ev->pixels[i] = (struct led_rgb){r : 255, g : 86, b : 0};
            }
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}
