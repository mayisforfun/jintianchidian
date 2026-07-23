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

volatile bool g_imu_ok = false;
volatile uint8_t g_mpu6050_who_am_i = 0;
volatile int16_t g_imu_accel_x = 0;
volatile int16_t g_imu_accel_y = 0;
volatile int16_t g_imu_accel_z = 0;
volatile int16_t g_imu_gyro_x = 0;
volatile int16_t g_imu_gyro_y = 0;
volatile int16_t g_imu_gyro_z = 0;
volatile uint32_t g_imu_read_count = 0;
volatile uint32_t g_imu_read_fail_count = 0;

static void update_imu_debug_values(const MPU6050RawData *imu_raw)
{
    g_imu_accel_x = imu_raw->accel_x;
    g_imu_accel_y = imu_raw->accel_y;
    g_imu_accel_z = imu_raw->accel_z;
    g_imu_gyro_x = imu_raw->gyro_x;
    g_imu_gyro_y = imu_raw->gyro_y;
    g_imu_gyro_z = imu_raw->gyro_z;
}

int main(void)
{
    LineFollowState line_state;
    MPU6050RawData imu_raw;
    bool imu_ok;
    uint8_t who_am_i = 0;
    uint8_t imu_sample_divider = 0;

    Board_init();
    LineFollow_init(&line_state);
    (void) MPU6050_readWhoAmI(&who_am_i);
    g_mpu6050_who_am_i = who_am_i;
    imu_ok = MPU6050_init();
    g_imu_ok = imu_ok;
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
                g_imu_ok = imu_ok;
                if (imu_ok) {
                    update_imu_debug_values(&imu_raw);
                    g_imu_read_count++;
                } else {
                    g_imu_read_fail_count++;
                }
            }
        }

        LineFollow_update(&line_state, &g_line_config, sensor_bits);
        Board_setMotorSpeed(line_state.left_speed, line_state.right_speed);

        Board_delayMs(1);
    }
}
