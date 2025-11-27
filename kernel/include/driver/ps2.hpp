#pragma once

#include <cstdint>

enum Key {
    KEY_ESCAPE = 0x01,
    KEY_SPACE = 0x39,
    KEY_ARROW_UP = 0x48,
    KEY_ARROW_DOWN = 0x50,
    KEY_ARROW_LEFT = 0x4b,
    KEY_ARROW_RIGHT = 0x4d,
    KEY_SHIFT_LEFT = 0x2A,
    KEY_SHIFT_RIGHT = 0x36,
    KEY_CONTROL = 0x1D,
    KEY_A = 0x1E,
    KEY_C = 0x2E,
    KEY_D = 0x20,
    KEY_R = 0x13,
    KEY_P = 0x19,
    KEY_S = 0x1F,
    KEY_W = 0x11,
    KEY_X = 0x2D,
    KEY_Z = 0x2C,
};

using key_listener_t = void(*)(uint8_t scancode);

void kb_register_listener(key_listener_t listener);

void kb_unregister_listener();

/**
 * Process any queued scancodes, forwarding them to the listener
 */
void kb_process_queue();

/**
 * Register interrupt on channel 1 for handling keypresses
 */
void kb_init();
