#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#include "modifiers.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* Combined left+right modifier bits */
#define MOD_SHIFT BIT(0)
#define MOD_CTRL  BIT(1)
#define MOD_ALT   BIT(2)
#define MOD_GUI   BIT(3)

/* HID keyboard usage page */
#define HID_USAGE_PAGE_KEY 0x07

/* HID modifier keycodes 0xE0–0xE7 */
#define HID_KEY_LCTRL   0xE0
#define HID_KEY_LSHIFT  0xE1
#define HID_KEY_LALT    0xE2
#define HID_KEY_LGUI    0xE3
#define HID_KEY_RCTRL   0xE4
#define HID_KEY_RSHIFT  0xE5
#define HID_KEY_RALT    0xE6
#define HID_KEY_RGUI    0xE7

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct modifier_state {
    uint8_t mods;
};

static struct modifier_state current_state;

static void set_indicator(lv_obj_t *box, bool active) {
    lv_obj_t *lbl = lv_obj_get_child(box, 0);
    if (active) {
        lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(box, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_black(), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
    }
}

static void update_widget(struct zmk_widget_modifiers *widget, struct modifier_state state) {
    set_indicator(lv_obj_get_child(widget->obj, 0), state.mods & MOD_SHIFT);
    set_indicator(lv_obj_get_child(widget->obj, 1), state.mods & MOD_CTRL);
    set_indicator(lv_obj_get_child(widget->obj, 2), state.mods & MOD_ALT);
    set_indicator(lv_obj_get_child(widget->obj, 3), state.mods & MOD_GUI);
}

static void modifier_update_cb(struct k_work *work) {
    struct zmk_widget_modifiers *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_widget(widget, current_state);
    }
}

K_WORK_DEFINE(modifier_update_work, modifier_update_cb);

static int keycode_state_changed_handler(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (!ev || ev->usage_page != HID_USAGE_PAGE_KEY) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_t mod = 0;
    switch (ev->keycode) {
    case HID_KEY_LSHIFT:
    case HID_KEY_RSHIFT:
        mod = MOD_SHIFT;
        break;
    case HID_KEY_LCTRL:
    case HID_KEY_RCTRL:
        mod = MOD_CTRL;
        break;
    case HID_KEY_LALT:
    case HID_KEY_RALT:
        mod = MOD_ALT;
        break;
    case HID_KEY_LGUI:
    case HID_KEY_RGUI:
        mod = MOD_GUI;
        break;
    default:
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->state) {
        current_state.mods |= mod;
    } else {
        current_state.mods &= ~mod;
    }

    k_work_submit(&modifier_update_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_modifiers, keycode_state_changed_handler);
ZMK_SUBSCRIPTION(widget_modifiers, zmk_keycode_state_changed);

/* x positions: 2, 34, 66, 98 — each box 28px wide, 4px gap, 2px margins */
static const lv_coord_t indicator_x[] = {2, 34, 66, 98};
static const char *const indicator_labels[] = {"SHFT", "CTRL", "ALT", "GUI"};

static lv_obj_t *make_indicator(lv_obj_t *parent, const char *text, lv_coord_t x) {
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_pos(box, x, 1);
    lv_obj_set_size(box, 28, 14);
    lv_obj_set_style_pad_all(box, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(box, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(box, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(box, 2, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(box);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_8, LV_PART_MAIN);
    lv_obj_center(lbl);
    lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);

    return box;
}

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 128, 16);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);

    for (int i = 0; i < 4; i++) {
        make_indicator(widget->obj, indicator_labels[i], indicator_x[i]);
    }

    sys_slist_append(&widgets, &widget->node);
    return 0;
}

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget) {
    return widget->obj;
}
