/*
 * Copyright (c) 2026 Yiancar
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/rgb_underglow_tick.h>
#include <zmk/keymap.h>
#include <zmk/rgb_underglow.h>

static K_MUTEX_DEFINE(indicator_mutex);
static bool is_capslock_on;
static bool is_layer1_on;

static int hid_indicator_change_listener(const zmk_event_t *eh);

ZMK_LISTENER(hid_indicator, hid_indicator_change_listener);
ZMK_SUBSCRIPTION(hid_indicator, zmk_hid_indicators_changed);
ZMK_SUBSCRIPTION(hid_indicator, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(hid_indicator, zmk_rgb_underglow_tick_event);

static void set_indicator_state(bool *indicator, bool active) {
    k_mutex_lock(&indicator_mutex, K_FOREVER);
    *indicator = active;
    zmk_rgb_underglow_set_output_override(is_layer1_on || is_capslock_on);
    k_mutex_unlock(&indicator_mutex);
}

static int hid_indicator_change_listener(const zmk_event_t *eh) {
    const struct zmk_hid_indicators_changed *hid_ev;
    struct zmk_rgb_underglow_tick_event *underglow_ev = as_zmk_rgb_underglow_tick_event(eh);

    if ((hid_ev = as_zmk_hid_indicators_changed(eh)) != NULL) {
        set_indicator_state(&is_capslock_on, hid_ev->indicators & BIT(1));
    } else if (as_zmk_layer_state_changed(eh) != NULL) {
        set_indicator_state(&is_layer1_on, zmk_keymap_layer_active(1));
    } else if ((underglow_ev = as_zmk_rgb_underglow_tick_event(eh)) != NULL) {
        k_mutex_lock(&indicator_mutex, K_FOREVER);
        if (is_capslock_on) {
            underglow_ev->pixels[66] = (struct led_rgb){.r = 5, .g = 2, .b = 1};
        } else {
            underglow_ev->pixels[66] = (struct led_rgb){.r = 0, .g = 0, .b = 0};
        }
        if (is_layer1_on) {
            underglow_ev->pixels[67] = (struct led_rgb){.r = 3, .g = 1, .b = 5};
        } else {
            underglow_ev->pixels[67] = (struct led_rgb){.r = 0, .g = 0, .b = 0};
        }
        k_mutex_unlock(&indicator_mutex);
    }
    return ZMK_EV_EVENT_BUBBLE;
}
