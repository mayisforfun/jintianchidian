#include "board_port.h"
#include "line_follow.h"
#include "mpu6050.h"

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
    MPU6050RawData imu_raw;
    bool imu_ok;
    uint8_t imu_sample_divider = 0;

    Board_init();
    LineFollow_init(&line_state);
    imu_ok = MPU6050_init();
    if (!imu_ok) {
        Board_setBuzzer(true);
    }

    while (1) {
        uint8_t sensor_bits = Board_readGray5();

        if (imu_ok) {
            imu_sample_divider++;
            if (imu_sample_divider >= 10U) {
                imu_sample_divider = 0;
                imu_ok = MPU6050_readRaw(&imu_raw);
            }
        }

        LineFollow_update(&line_state, &g_line_config, sensor_bits);
        Board_setMotorSpeed(line_state.left_speed, line_state.right_speed);

        Board_delayMs(1);
    }
}
