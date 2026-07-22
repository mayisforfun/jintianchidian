# MSPM0G3507 五路灰度循迹示例

适用目标：CCS Theia / Code Composer Studio，TI MSPM0G3507，天狼星开发板系列，五路数字灰度模块，TB6612 小车差速驱动。

## 文件

- `main.c`：1 ms 循环读取灰度、计算循迹、输出电机速度。
- `line_follow.c/.h`：五路灰度识别和 PD 循迹算法。
- `board_port.c/.h`：MSPM0 GPIO、PWM、电机方向接口。

## SysConfig 需要建立的外设

### 1. 五路灰度 GPIO 输入

建立 5 个 GPIO 输入：

- `L2`：最左，A26
- `L1`：左中，A27
- `C`：中间，A24
- `R1`：右中，A25
- `R2`：最右，B24

代码已经在 `board_port.h` 里按 `D:\qq\IO口对应.xlsx` 写好了默认引脚。SysConfig 的作用是把这些实际芯片引脚初始化成 GPIO 输入。

若你的灰度模块检测到黑线时输出高电平，把 `board_port.h` 中的：

```c
#define GRAY_BLACK_IS_LOW 1
```

改成：

```c
#define GRAY_BLACK_IS_LOW 0
```

### 2. TB6612 电机方向 GPIO 输出

建立 4 个 GPIO 输出：

- `AIN1`：B23
- `AIN2`：B26
- `BIN1`：B08
- `BIN2`：B09
- `STBY`：表格里未看到明确分配，可以直接接 3.3 V

建议接法：

```text
TB6612 AIN1 -> B23
TB6612 AIN2 -> B26
TB6612 BIN1 -> B08
TB6612 BIN2 -> B09
TB6612 STBY -> 3.3 V，或另接一个 GPIO 输出高电平
```

默认代码把 A 通道当左电机，把 B 通道当右电机。如果实际前进方向反了，交换对应电机的 `AIN1/AIN2` 或 `BIN1/BIN2`，也可以改 `board_port.c` 里的 `set_left_direction()` / `set_right_direction()`。

### 3. 两路 PWM

TB6612 的 `PWMA` 控制 A 通道速度，`PWMB` 控制 B 通道速度。给左右电机各配置一路 PWM，PWM 周期建议 1000 计数，对应代码里的：

```c
#define MOTOR_PWM_PERIOD_COUNTS 1000U
```

然后在 `board_port.c` 顶部把以下宏改成你 SysConfig 生成的名字：

```c
#define LEFT_PWM_INST PWM_0_INST
#define LEFT_PWM_CC_INDEX DL_TIMER_CC_0_INDEX
#define RIGHT_PWM_INST PWM_1_INST
#define RIGHT_PWM_CC_INDEX DL_TIMER_CC_0_INDEX
```

建议接法：

```text
TB6612 PWMA -> MSPM0 左电机 PWM
TB6612 PWMB -> MSPM0 右电机 PWM
```

如果你用同一个 Timer 的两个 PWM 通道，`LEFT_PWM_INST` 和 `RIGHT_PWM_INST` 可以相同，但 `CC_INDEX` 应该不同。

## 灰度位定义

`Board_readGray5()` 返回 5 bit：

```text
传感器：L2  L1  C   R1  R2
bit：   0   1   2   3   4
```

黑线被识别到时对应 bit 为 1。例如：

- 只有 `C` 为 1：中间压线，直行。
- `L1 + C` 或 `L2 + L1` 为 1：线偏左，小车左修正。
- `C + R1` 或 `R1 + R2` 为 1：线偏右，小车右修正。
- 五路全为 0：丢线，按上一次误差原地找线。

## 调参顺序

1. 先把车架垫起来，确认左右电机方向和 PWM 占空比正常。
2. 慢速测试：`base_speed = 300`，`max_speed = 650`。
3. 如果车左右摆动大，降低 `kp_q10` 或升高 `kd_q10`。
4. 如果车转弯跟不上，提高 `kp_q10` 或降低 `base_speed`。
5. 如果丢线后原地找线太猛，降低 `lost_turn_speed`。

当前默认参数在 `main.c`：

```c
.base_speed = 420,
.max_speed = 850,
.kp_q10 = 164,
.kd_q10 = 225,
.lost_turn_speed = 320,
```

`kp_q10` 和 `kd_q10` 是 Q10 定点数，实际系数 = 参数 / 1024。例如 `164 / 1024 = 0.160`。

## 接入 CCS 工程

1. 新建 MSPM0G3507 空工程或基于 TI empty 示例工程。
2. 用 SysConfig 配好 `GRAY`、`MOTOR`、左右 PWM。
3. 把本目录下 `.c` 和 `.h` 文件加入工程。
4. 若工程已有 `main.c`，用这里的 `main.c` 逻辑替换主循环即可。
5. 编译时报某个宏未定义时，优先检查 SysConfig 里外设和引脚名称是否与本文一致。
