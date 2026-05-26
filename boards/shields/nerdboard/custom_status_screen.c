#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/display/widgets/battery_status.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/output_status.h>
#include "widgets/modifiers.h"
#endif

static struct zmk_widget_battery_status battery_status_widget;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static struct zmk_widget_layer_status layer_status_widget;
static struct zmk_widget_output_status output_status_widget;
static struct zmk_widget_modifiers modifiers_widget;
#endif

/*
 * Right (central) layout on 128×64 SSD1306:
 *   y= 0  [OUT]               [BAT]
 *   y=16  [      Layer Name       ]
 *   y=48  [SHFT] [CTRL] [ALT] [GUI]
 *
 * Left (peripheral) layout:
 *   y= 0                      [BAT]
 */
lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
                 LV_ALIGN_TOP_LEFT, 0, 0);
#endif

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
                 LV_ALIGN_TOP_RIGHT, 0, 0);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget),
                 LV_ALIGN_TOP_MID, 0, 16);

    zmk_widget_modifiers_init(&modifiers_widget, screen);
    lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget),
                 LV_ALIGN_BOTTOM_LEFT, 0, 0);
#endif

    return screen;
}
