#define DT_DRV_COMPAT zmk_input_processor_rotate

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <zmk/input/processors.h>

/* -15 degree rotation (scaled by 1000):
 *   cos(-15°) =  966 / 1000
 *   sin(-15°) = -259 / 1000
 *
 *   x' =  cos*x - sin*y =  966*x + 259*y
 *   y' =  sin*x + cos*y = -259*x + 966*y
 *
 * X is buffered when it arrives and suppressed; both axes are emitted
 * with corrected values once Y arrives.
 */
#define ROT_COS  966
#define ROT_SIN (-259)

struct rotate_data {
    int32_t buffered_x;
    bool x_ready;
    bool injecting;
};

static int rotate_handle_event(const struct device *dev,
                                struct input_event *event,
                                uint32_t param1, uint32_t param2,
                                struct zmk_input_processor_remainder *remainder) {
    struct rotate_data *data = dev->data;

    if (event->type != INPUT_EV_REL) {
        return 0;
    }

    if (event->code == INPUT_REL_X) {
        if (data->injecting) {
            data->injecting = false;
            return 0;
        }
        data->buffered_x = event->value;
        data->x_ready = true;
        return -EAGAIN; /* suppress original X; rotated X re-injected when Y arrives */
    }

    if (event->code == INPUT_REL_Y && data->x_ready) {
        int32_t rx = data->buffered_x;
        int32_t ry = event->value;

        int32_t new_x = (ROT_COS * rx - ROT_SIN * ry) / 1000;
        int32_t new_y = (ROT_SIN * rx + ROT_COS * ry) / 1000;

        data->x_ready = false;
        data->injecting = true;
        input_report_rel(event->dev, INPUT_REL_X, new_x, false, K_NO_WAIT);

        event->value = new_y;
    }

    return 0;
}

static const struct zmk_input_processor_driver_api rotate_api = {
    .handle_event = rotate_handle_event,
};

#define ROTATE_INST(n)                                                          \
    static struct rotate_data rotate_data_##n;                                  \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &rotate_data_##n, NULL,               \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,     \
                          &rotate_api);

DT_INST_FOREACH_STATUS_OKAY(ROTATE_INST)
