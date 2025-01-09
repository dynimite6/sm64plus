#include <stdbool.h>
#include <ultra64.h>

#include "controller_api.h"

#ifdef TARGET_WEB
#include "controller_emscripten_keyboard.h"
#endif

#include "game/settings.h"

static int keyboard_buttons_down;

static long mouse_x;
static long mouse_y;
static s8 mouse_wheel;

static int keyboard_mapping[15][2];

static int keyboard_map_scancode(int scancode) {
    int ret = 0;
    for (size_t i = 0; i < sizeof(keyboard_mapping) / sizeof(keyboard_mapping[0]); i++) {
        if (keyboard_mapping[i][0] == scancode) {
            ret |= keyboard_mapping[i][1];
        }
    }
    return ret;
}

bool keyboard_on_key_down(int scancode) {
    int mapped = keyboard_map_scancode(scancode);
    keyboard_buttons_down |= mapped;
    return mapped != 0;
}

bool keyboard_on_key_up(int scancode) {
    int mapped = keyboard_map_scancode(scancode);
    keyboard_buttons_down &= ~mapped;
    return mapped != 0;
}

void keyboard_on_all_keys_up(void) {
    keyboard_buttons_down = 0;
}

void keyboard_on_mouse_move(long x, long y) {
    mouse_x = x;
    mouse_y = y;
}

void keyboard_on_mouse_press(s8 left, s8 right, s8 middle, s8 wheel) {
    if (!configMouseCam)
        return;
        
    if (left > 0)
        keyboard_buttons_down |= configMouseLeft;
    if (left < 0)
        keyboard_buttons_down &= ~configMouseLeft;

    if (right > 0)
        keyboard_buttons_down |= configMouseRight;
    if (right < 0)
        keyboard_buttons_down &= ~configMouseRight;

    if (middle > 0)
        keyboard_buttons_down |= configMouseMiddle;
    if (middle < 0)
        keyboard_buttons_down &= ~configMouseMiddle;

    if (wheel < 0) {
        keyboard_buttons_down |= configMouseWheelDown;   
        mouse_wheel = wheel;
    }
    if (wheel > 0) {
        keyboard_buttons_down |= configMouseWheelUp;
        mouse_wheel = wheel;
    }
}

static void set_keyboard_mapping(int index, int mask, int scancode) {
    keyboard_mapping[index][0] = scancode;
    keyboard_mapping[index][1] = mask;
}

static void keyboard_init(void) {
    int i = 0;

    set_keyboard_mapping(i++, 0x80000,      configKeyStickUp);
    set_keyboard_mapping(i++, 0x10000,      configKeyStickLeft);
    set_keyboard_mapping(i++, 0x40000,      configKeyStickDown);
    set_keyboard_mapping(i++, 0x20000,      configKeyStickRight);
    set_keyboard_mapping(i++, A_BUTTON,     configKeyA);
    set_keyboard_mapping(i++, B_BUTTON,     configKeyB);
    set_keyboard_mapping(i++, Z_TRIG,       configKeyZ);
    if (configImprovedCamera) {
        set_keyboard_mapping(i++, U_CBUTTONS,   configKeyCUp);
        set_keyboard_mapping(i++, 0x100000,   configKeyCLeft);
        set_keyboard_mapping(i++, D_CBUTTONS,   configKeyCDown);
        set_keyboard_mapping(i++, 0x200000,   configKeyCRight);
    }
    else {
        set_keyboard_mapping(i++, U_CBUTTONS,   configKeyCUp);
        set_keyboard_mapping(i++, L_CBUTTONS,   configKeyCLeft);
        set_keyboard_mapping(i++, D_CBUTTONS,   configKeyCDown);
        set_keyboard_mapping(i++, R_CBUTTONS,   configKeyCRight);
    }
    set_keyboard_mapping(i++, L_TRIG,       configKeyL);
    set_keyboard_mapping(i++, R_TRIG,       configKeyR);
    set_keyboard_mapping(i++, START_BUTTON, configKeyStart);
    set_keyboard_mapping(i++, 0x1000000,    configKeyWalk);

#ifdef TARGET_WEB
    controller_emscripten_keyboard_init();
#endif
}

static void keyboard_read(OSContPad *pad) {
    // Camera movement with mouse
    if (configMouseCam) {
        if (mouse_x != 0)
            pad->stick2_x = mouse_x*configMouseSensitivity;
        if (mouse_y != 0)
            pad->stick2_y = mouse_y*configMouseSensitivity;
        if (mouse_wheel < -1) {
            keyboard_buttons_down &= ~configMouseWheelDown;
            mouse_wheel = 0;
        }
        if (mouse_wheel > 1) {
            keyboard_buttons_down &= ~configMouseWheelUp;
            mouse_wheel = 0;
        }
        mouse_wheel *= 2;
    }

    pad->button |= keyboard_buttons_down;
    if ((keyboard_buttons_down & 0x30000) == 0x10000) {
        pad->stick_x = (keyboard_buttons_down & 0x1000000) ? -32 : -128;
    }
    if ((keyboard_buttons_down & 0x30000) == 0x20000) {
        pad->stick_x = (keyboard_buttons_down & 0x1000000) ? 32 : 127;
    }
    if ((keyboard_buttons_down & 0xc0000) == 0x40000) {
        pad->stick_y = (keyboard_buttons_down & 0x1000000) ? -32 : -128;
    }
    if ((keyboard_buttons_down & 0xc0000) == 0x80000) {
        pad->stick_y = (keyboard_buttons_down & 0x1000000) ? 32 : 127;
    }

    if ((keyboard_buttons_down & 0x300000) == 0x100000) {
        pad->stick2_x = -80;
    }
    if ((keyboard_buttons_down & 0x300000) == 0x200000) {
        pad->stick2_x = 80;
    }
    if ((keyboard_buttons_down & 0xc00000) == 0x400000) {
        pad->stick2_y = -80;
    }
    if ((keyboard_buttons_down & 0xc00000) == 0x800000) {
        pad->stick2_y = 80;
    }
}

struct ControllerAPI controller_keyboard = {
    keyboard_init,
    keyboard_read,
    NULL,
    NULL
};