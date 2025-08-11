// Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "joshuaauger.h"
#include "socd_cleaner.h"
#include "sentence_case.h"
#include "rgb_utils.h"

socd_cleaner_t socd_v = {{KC_W, KC_S}, SOCD_CLEANER_LAST};
socd_cleaner_t socd_h = {{KC_A, KC_D}, SOCD_CLEANER_LAST};


//Set power-up defaults for lighting
void keyboard_post_init_user(void) {
    //Initialize default user-configurable HSV values
    user_hsv.h = 115;
    user_hsv.s = 255;
    user_hsv.v = 255;

    //Initialize bools for function layer illumination behavior
    do_reset = false;
    first_press = true;

    //Set the default lighting mode for the keyboard when it powers up. See rgb_utils.h for all lighting modes available
    LIGHTING_MODE = QMK_FANCY_SIDE;
}

//Write current QMK RGB settings to EEPROM
void save_qmk_rgb_settings(void) {
    rgb_matrix_mode(rgb_matrix_get_mode());
    rgb_matrix_set_speed(rgb_matrix_get_speed());
    rgb_matrix_sethsv(rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val());
}

// Sets Row 4 (caps row) to red when caps lock is enabled
void set_caps_row_led(bool caps_on) {
    if (caps_on) {
        for (uint8_t curr_led = 0; curr_led < CAPS_LED_NUM; curr_led++) {
            rgb_matrix_set_color(LEDS_LIST_CAPS[curr_led], RGB_RED);
        }
    } else {
        for (uint8_t curr_led = 0; curr_led < CAPS_LED_NUM; curr_led++) {
            rgb_matrix_set_color(LEDS_LIST_CAPS[curr_led], RGB_BLACK);
        }
    }
}

void set_rgb_mode(bool clockwise) {
    //Not writing to eeprom, so user will need to manually save settings with RCTL + knob click
    if (clockwise) {
        rgb_matrix_step_noeeprom();
    }
    else {
        rgb_matrix_step_reverse_noeeprom();
    }
}

void set_rgb_speed(bool clockwise) {
    if (clockwise) {
        rgb_matrix_increase_speed_noeeprom();
    }
    else {
        rgb_matrix_decrease_speed_noeeprom();
    }
}

void adjust_user_lighting_hsv(bool clockwise, uint8_t hue, uint8_t sat, uint8_t val) {
    if (clockwise) {
        //Check bounds for all except hue
        user_hsv.h += hue;

        if (user_hsv.s + sat > 255) {
            user_hsv.s = 255;
        }
        else {
            user_hsv.s += sat;
        }
        if (user_hsv.v + val > 255) {
            user_hsv.v = 255;
        }
        else {
            user_hsv.v += val;
        }
    }
    else {
        user_hsv.h -= hue;

        if (user_hsv.s - sat < 0) {
            user_hsv.s = 0;
        }
        else {
            user_hsv.s -= sat;
        }
        if (user_hsv.v - val < 0) {
            user_hsv.v = 0;
        }
        else {
            user_hsv.v -= val;
        }
    }
}

void adjust_qmk_effect_hsv(bool clockwise, bool adj_hue, bool adj_sat, bool adj_val) {
    //Adjusts QMK lighting (not user-defined side lighting) by the default RGBLIGHT_*_STEP values, defined in config.h
    if (clockwise) {
        if (adj_hue) {
            rgb_matrix_increase_hue_noeeprom();
        }
        else if (adj_sat) {
            rgb_matrix_increase_sat_noeeprom();
        }
        else if (adj_val) {
            rgb_matrix_increase_val_noeeprom();
        }
    }
    else {
        if (adj_hue) {
            rgb_matrix_decrease_hue_noeeprom();
        }
        else if (adj_sat) {
            rgb_matrix_decrease_sat_noeeprom();
        }
        else if (adj_val) {
            rgb_matrix_decrease_val_noeeprom();
        }
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    //If we're above the base layer then do special illumination for certain keys
    if (get_highest_layer(layer_state) > 0) {
        uint8_t curr_layer = get_highest_layer(layer_state);

        //Disable LEDs temporarily while Fn is pressed
        if (first_press) {
            last_hsv = rgb_matrix_get_hsv();
            rgb_matrix_sethsv_noeeprom(HSV_BLACK);
            first_press = false;
            do_reset = true;
        }

        // Illuminate specific layer 1 and RGB keys when Fn is pressed
        switch (curr_layer) {
            case 1:
            //Media controls
            rgb_matrix_set_color(LED_S, RGB_GOLD);
            rgb_matrix_set_color(LED_A, RGB_SPRINGGREEN);
            rgb_matrix_set_color(LED_D, RGB_SPRINGGREEN);
            rgb_matrix_set_color(LED_W, RGB_GOLD);

            //Bootloader key
            rgb_matrix_set_color(LED_BSLS, RGB_MAGENTA);

            //RGB controls
            rgb_matrix_set_color(LED_LSFT, RGB_RED);
            rgb_matrix_set_color(LED_LCTL, RGB_GREEN);
            rgb_matrix_set_color(LED_LWIN, RGB_BLUE);
            rgb_matrix_set_color(LED_RSFT, RGB_PINK);
            rgb_matrix_set_color(LED_RCTL, RGB_AZURE);

            //Virtual numpad
            rgb_matrix_set_color(LED_T, RGB_WHITE);
            rgb_matrix_set_color(LED_Y, RGB_WHITE);
            rgb_matrix_set_color(LED_U, RGB_WHITE);
            rgb_matrix_set_color(LED_G, RGB_WHITE);
            rgb_matrix_set_color(LED_H, RGB_WHITE);
            rgb_matrix_set_color(LED_J, RGB_WHITE);
            rgb_matrix_set_color(LED_V, RGB_WHITE);
            rgb_matrix_set_color(LED_B, RGB_WHITE);
            rgb_matrix_set_color(LED_N, RGB_WHITE);

            //Num lock - white when off, red when on
            if (host_keyboard_led_state().num_lock) {
                rgb_matrix_set_color(LED_P, RGB_RED);
            }
            else {
                rgb_matrix_set_color(LED_P, RGB_WHITE);
            }

            //Caps word key
            rgb_matrix_set_color(LED_CAPS, RGB_WHITE);
        }
    }
    //Back in layer 0
    else {
        //Restore RGB settings when out of function layer
        if (do_reset) {
            rgb_matrix_sethsv_noeeprom(last_hsv.h, last_hsv.s, last_hsv.v);
            first_press = true;
            do_reset = false;
        }

        //Turn off top LEDs and leave QMK RGB effects on the side configurable as normal with RGB mode buttons
        //Unfortunately these options do not persist after power loss due to the fact that the rgb_matrix_set function does not write to EEPROM
        //Static user color and mode will have to be reset manually in that case
        if (LIGHTING_MODE == QMK_FANCY_SIDE) {
            for (uint8_t i = 0; i < NUM_KEYS; i++) {
                rgb_matrix_set_color(LEDS_NOT_SIDE[i], RGB_BLACK);
            }
        }
        else if (LIGHTING_MODE == USER_SOLID_SIDE) {
            RGB user_rgb = hsv_to_rgb(user_hsv);
            for (uint8_t i = 0; i < SIDE_LEDS_NUM; i++) {
                rgb_matrix_set_color(LEFT_SIDE_RGB[i], user_rgb.r, user_rgb.g, user_rgb.b);
                rgb_matrix_set_color(RIGHT_SIDE_RGB[i], user_rgb.r, user_rgb.g, user_rgb.b);
            }
        }

        //Caps lock LED
        if (host_keyboard_led_state().caps_lock) {
            set_caps_row_led(true);
        } else {
            if (rgb_matrix_get_flags() == LED_FLAG_NONE) {
                set_caps_row_led(false);
            }
        }
    }

    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_sentence_case(keycode, record)) { return false; }
    if (!process_socd_cleaner(keycode, record, &socd_v)) { return false; }
    if (!process_socd_cleaner(keycode, record, &socd_h)) { return false; }
    switch (keycode) {
        //Credit to GitHub user @jwhurley1 for this trick
        //Relies on the volume knob press being bound to KC_MUTE
        //Incidentally, any other keys set to KC_MUTE will also trigger this if CTRL is pressed simultaneously
        case KC_MUTE:
            if (record -> event.pressed) {
                if (get_mods() & MOD_BIT(KC_LCTL)) {
                    //Toggle main keys lighting and solid user-defined side lighting
                    if (LIGHTING_MODE == USER_SOLID_SIDE) {
                        LIGHTING_MODE = QMK_FANCY_ALL;
                        //Set back to previous HSV setting when user option is turned off
                        rgb_matrix_sethsv_noeeprom(last_hsv.h, last_hsv.s, last_hsv.v);
                    }
                    else {
                        LIGHTING_MODE = USER_SOLID_SIDE;
                        //Retain current hue and sat, but set brightness to 0 while in this mode
                        last_hsv = rgb_matrix_get_hsv();
                        rgb_matrix_sethsv_noeeprom(HSV_BLACK);
                    }
                    return false;
                }
                else if (get_mods() & MOD_BIT(KC_LALT)) {
                    //Check for user side lighting enabled and disable if necessary
                    if (LIGHTING_MODE == USER_SOLID_SIDE) {
                        //Make sure we set the last HSV value back otherwise we end up with nothing if switching from user -> fancy mode
                        rgb_matrix_sethsv_noeeprom(last_hsv.h, last_hsv.s, last_hsv.v);
                    }

                    //Toggle main keys lighting but leave the built-in QMK RGB effects active on the side
                    if (LIGHTING_MODE == QMK_FANCY_SIDE) {
                        LIGHTING_MODE = QMK_FANCY_ALL;
                    }
                    else {
                        LIGHTING_MODE = QMK_FANCY_SIDE;
                    }
                    return false;
                }
                else if (get_mods() & MOD_BIT(KC_LSFT)) {
                    //Toggle RGB on/off
                    rgb_matrix_toggle();
                    return false;
                }
                //Save the current QMK effect HSV settings to EEPROM
                else if (get_mods() & MOD_BIT(KC_RCTL)) {
                    save_qmk_rgb_settings();
                    return false;
                }
                //If KC_MUTE received but CTRL or ALT (exclusively) not pressed
                return true;
            }
        default:
            return true; // Process all other keycodes normally
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    uint8_t current_mods = get_mods();

    //Adjust RGB settings depending on modifier keys and lighting type active at time of encoder action
    if (LIGHTING_MODE == USER_SOLID_SIDE) {
        if ((current_mods & MOD_BIT(KC_LCTL)) && (current_mods & MOD_BIT(KC_LALT))) {
            adjust_user_lighting_hsv(clockwise, 0, 0, RGBLIGHT_VAL_STEP);
            return false;
        }
        else if (current_mods & MOD_BIT(KC_LCTL)) {
            adjust_user_lighting_hsv(clockwise, RGBLIGHT_HUE_STEP, 0, 0);
            return false;
        }
        else if (current_mods & MOD_BIT(KC_LALT)) {
            adjust_user_lighting_hsv(clockwise, 0, RGBLIGHT_SAT_STEP, 0);
            return false;
        }
        else {
            if (clockwise) {
                tap_code16(KC_VOLU);
                return false;
            } else {
                tap_code16(KC_VOLD);
                return false;
            }
        }
    }
    //Not in user defined solid mode
    else {
        if (current_mods & MOD_BIT(KC_LSFT)) {
            set_rgb_mode(clockwise);
            return false;
        }
        else if (current_mods & MOD_BIT(KC_RSFT)) {
            set_rgb_speed(clockwise);
            return false;
        }
        //HSV controls for built in QMK effects. Changes are not saved until the user presses RCTL + Knob click to reduce EEPROM writes
        if ((current_mods & MOD_BIT(KC_LCTL)) && (current_mods & MOD_BIT(KC_LALT))) {
            adjust_qmk_effect_hsv(clockwise, false, false, true);
            return false;
        }
        else if (current_mods & MOD_BIT(KC_LCTL)) {
            adjust_qmk_effect_hsv(clockwise, true, false, false);
            return false;
        }
        else if (current_mods & MOD_BIT(KC_LALT)) {
            adjust_qmk_effect_hsv(clockwise, false, true, false);
            return false;
        }
        else {
            if (clockwise) {
                tap_code16(KC_VOLU);
                return false;
            } else {
                tap_code16(KC_VOLD);
                return false;
            }
        }
    }

    return true;
}
