# Version Log

## 2026-07-23 - IMU Watch debug variables

Commit: pending

Status:
- Added global volatile MPU6050 debug variables for CCS Watch / Expressions.
- Intended for silent classroom testing without relying on the buzzer.

Changed:
- `main.c`
  - Added `g_mpu6050_who_am_i`.
  - Added `g_imu_ok`.
  - Added `g_imu_accel_x/y/z`.
  - Added `g_imu_gyro_x/y/z`.
  - Added `g_imu_read_count`.
  - Added `g_imu_read_fail_count`.

How to test:
- Watch `g_mpu6050_who_am_i`; expected value is `0x68`.
- Watch `g_imu_read_count`; it should keep increasing after initialization.
- Tilt the MPU6050 and watch `g_imu_accel_x/y/z` change.
- Rotate the MPU6050 and watch `g_imu_gyro_x/y/z` change.

Notes:
- If `g_mpu6050_who_am_i` stays `0` or `g_imu_ok` is `false`, check I2C wiring and SysConfig.
- MPU6050 uses `PA17/I2C1_SCL` and `PA16/I2C1_SDA` from `D:\qq\IO口对应.xlsx`.

## 2026-07-23 - MPU6050 driver and line-follow fixes

Commit: `0fc0c97`

Status:
- Pushed to `origin/master`.

Changed:
- Added `mpu6050.c/.h`.
- Added MPU6050 initialization and 10 ms raw data reading in `main.c`.
- Fixed `lost_turn_speed` overflow by changing it from `uint8_t` to `uint16_t`.
- Added gray sensor majority sampling.
- Added invalid non-contiguous gray sensor mask handling.
- Made PWM compare polarity configurable.
- Documented SysConfig requirements.

## 2026-07-22 - Initial line-follow example

Commit: `f9feff0`

Status:
- Initial pushed version.

Changed:
- Added five-channel gray sensor line-follow example.
- Added TB6612 board port layer.
- Added pin mapping documentation.
