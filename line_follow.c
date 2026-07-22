#include "line_follow.h"

#define BIT_SENSOR_L2 (1U << 0)
#define BIT_SENSOR_L1 (1U << 1)
#define BIT_SENSOR_C  (1U << 2)
#define BIT_SENSOR_R1 (1U << 3)
#define BIT_SENSOR_R2 (1U << 4)

static uint16_t clamp_u16(int32_t value, uint16_t max_value)
{
    if (value < 0) {
        return 0;
    }
    if (value > (int32_t) max_value) {
        return max_value;
    }
    return (uint16_t) value;
}

void LineFollow_init(LineFollowState *state)
{
    state->sensor_bits = 0;
    state->position = 0;
    state->error = 0;
    state->last_error = 0;
    state->line_seen = false;
    state->left_speed = 0;
    state->right_speed = 0;
}

void LineFollow_update(LineFollowState *state,
                       const LineFollowConfig *config,
                       uint8_t sensor_bits)
{
    static const int16_t weights[LINE_SENSOR_COUNT] = {-2000, -1000, 0, 1000, 2000};
    int32_t weighted_sum = 0;
    uint8_t active_count = 0;

    sensor_bits &= 0x1FU;
    state->sensor_bits = sensor_bits;

    for (uint8_t i = 0; i < LINE_SENSOR_COUNT; i++) {
        if ((sensor_bits & (1U << i)) != 0U) {
            weighted_sum += weights[i];
            active_count++;
        }
    }

    if (active_count == 0U) {
        state->line_seen = false;
        if (state->last_error >= 0) {
            state->left_speed = config->lost_turn_speed;
            state->right_speed = -(int16_t) config->lost_turn_speed;
        } else {
            state->left_speed = -(int16_t) config->lost_turn_speed;
            state->right_speed = config->lost_turn_speed;
        }
        return;
    }

    state->line_seen = true;
    state->position = (int16_t) (weighted_sum / active_count);
    state->error = state->position;

    int16_t derivative = state->error - state->last_error;
    int32_t correction = ((int32_t) config->kp_q10 * state->error +
                          (int32_t) config->kd_q10 * derivative) / 1024;
    state->last_error = state->error;

    state->left_speed = (int16_t) clamp_u16((int32_t) config->base_speed + correction,
                                            config->max_speed);
    state->right_speed = (int16_t) clamp_u16((int32_t) config->base_speed - correction,
                                             config->max_speed);

    if ((sensor_bits == BIT_SENSOR_L2) || (sensor_bits == (BIT_SENSOR_L2 | BIT_SENSOR_L1))) {
        state->left_speed = config->base_speed / 3;
        state->right_speed = config->max_speed;
    } else if ((sensor_bits == BIT_SENSOR_R2) || (sensor_bits == (BIT_SENSOR_R2 | BIT_SENSOR_R1))) {
        state->left_speed = config->max_speed;
        state->right_speed = config->base_speed / 3;
    } else if (sensor_bits == (BIT_SENSOR_L2 | BIT_SENSOR_L1 | BIT_SENSOR_C |
                              BIT_SENSOR_R1 | BIT_SENSOR_R2)) {
        state->left_speed = config->base_speed / 2;
        state->right_speed = config->base_speed / 2;
    }
}
