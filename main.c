#include "board_port.h"
#include "line_follow.h"

static const LineFollowConfig g_line_config = {
    .base_speed = 420,
    .max_speed = 850,
    .kp_q10 = 164,
    .kd_q10 = 225,
    .lost_turn_speed = 320,
};

int main(void)
{
    LineFollowState line_state;

    Board_init();
    LineFollow_init(&line_state);

    while (1) {
        uint8_t sensor_bits = Board_readGray5();

        LineFollow_update(&line_state, &g_line_config, sensor_bits);
        Board_setMotorSpeed(line_state.left_speed, line_state.right_speed);

        Board_delayMs(1);
    }
}
