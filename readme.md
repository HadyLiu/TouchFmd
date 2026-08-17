# TouchFmd

面向 **FMD 8 位 MCU** 的电容触摸固件：电荷转移 QTouch 与 ADC CVD 自电容。  
目标是可商用、抗干扰、RAM 占用低，引脚与算法参数全部宏定义，便于移植。

触摸后信号 **S 下降**，按下判据 `D = max(B − S, 0)`。每路独立保存 S/B/D/N/P，阈值与消抖也按通道配置。

许可证：[Apache License 2.0](LICENSE)

---

## 方案一览

| 目录 | 芯片 | 采集原理 | 外接 Cs | 通道数 | 适用 |
|------|------|----------|---------|--------|------|
| [`qTouchIO_FT62F21x`](qTouchIO_FT62F21x/) | FT62F21x | GPIO 电荷转移，计次数到约 1/2 VDD | 要（约 22 nF） | 1 | 无 ADC 或资源极紧 |
| [`qTouchIO_FT61EC2x`](qTouchIO_FT61EC2x/) | FT61EC2x | 同上 | 要 | 2 | IO 方案，SNS 可为非模拟脚 |
| [`qTouchADC_FT61EC2x`](qTouchADC_FT61EC2x/) | FT61EC2x | 固定少量转移后 10 位 ADC 读 Cs | 要 | 2 | SNS 须为 ANx |
| [`mTouchADC_FT61EC2x`](mTouchADC_FT61EC2x/) | FT61EC2x | 片内 CVD，C_hold=1/4VDD | **不要** | 2 | 多路、少外围、低功耗 |

选型建议：

1. **多路、少外围** → `mTouchADC_FT61EC2x`
2. **已有 SNS+SMP+Cs 板** → `qTouchADC_FT61EC2x`（SNS 为 ANx）或 `qTouchIO_FT61EC2x`（纯 GPIO）
3. **FT62F21x** → 只用 `qTouchIO_FT62F21x`

各方案细节见对应目录下的 `reademe.md`。

---

## 仓库结构

```text
TouchFmd/
├── qTouchIO_FT62F21x/     # GPIO QTouch，FT62F21x
├── qTouchIO_FT61EC2x/     # GPIO QTouch，FT61EC2x 两路
├── qTouchADC_FT61EC2x/    # ADC QTouch，FT61EC2x 两路
├── mTouchADC_FT61EC2x/    # 两路 ADC CVD，FT61EC2x
├── clearVxx.bat           # 清理 IDE 中间文件
└── LICENSE
```

每个方案都是独立工程，典型文件：

| 文件 | 职责 |
|------|------|
| `*_cfg.h` | 通道表、算法参数（不含寄存器） |
| `*_hal.c/h` | 硬件：GPIO / ADC / 中断；**移植芯片主要改这里** |
| `*_acq.c/h` | 采集驱动：电荷转移或 CVD 时序 |
| `*_key.c/h` | 滤波、基线、判决、校准 |
| `main.c` | 应用入口：扫描触摸，PA7/PA6 低有效指示通道1/2 |
| `touchdeg_main.c` | 调试入口：PA2 软串口约 9600 上报 |
| `soft_uart.c/h` | TouchDeg 软串口（无库除法打数字） |
| `main.prj` / `TouchDeg.prj` | FMD IDE 工程 |

---

## 硬件要点

### QTouch（IO / ADC）

SNS + SMP + 外接 Cs。手指靠近 → Cx 变大 → 每次转移电荷更多 → **S 下降**。

```text
  [铜箔 Cx]──[Rs ≈ 1 kΩ]──┬── GPIO SNS
                          │
                         Cs ≈ 22 nF NPO/C0G
                          │
                          └── GPIO SMP
```

FT61EC2x 两路 QTouch IO（只改 `qtouch_cfg.h` 通道表）：

```c
#define QT_CH0_SNS_IO PC2
#define QT_CH0_SNS_DIR TRISC2
#define QT_CH0_SMP_IO PC3
#define QT_CH0_SMP_DIR TRISC3
#define QT_CH1_SNS_IO PC1
#define QT_CH1_SNS_DIR TRISC1
#define QT_CH1_SMP_IO PC4
#define QT_CH1_SMP_DIR TRISC4
```

| 通道 | SNS | SMP |
|------|-----|-----|
| CH0 | PC2(AN6) | PC3 |
| CH1 | PC1(AN5) | PC4 |

IO 与 ADC 同一套脚。手册：**AN5=PC1，AN6=PC2，PC4 无 ADC**（作 SMP）。

指示：PA7=通道1、PA6=通道2，**低有效**。日志 TX：PA2。Cs 用 NPO/C0G；Rs 约 1 kΩ。

### mTouch CVD

每路铜箔经 Rs 接到 `ANx`。片内 C_hold 与 Cx 分压，**无外接 22 nF**。  
FT61EC2x 用内部 **1/4VDD (AN7)** 预充 C_hold，不占 GPIO。

默认两路：PC2/AN6、PC1/AN5。改通道只动 `mtouch_cfg.h` 的 `MT_CHn_ADC` / `MT_CHn_BIT` 和 `MT_CH_COUNT`。

### 公共约束

- Cs 用 NPO/C0G；Rs 约 1 kΩ
- 上电 `WPUC = 0` 关闭上拉；采集热路径不再逐拍清 WPU
- FT61EC2x：**PA0 为 ISP CLK**；勿占用 PA7/PA6（指示）和 PA2（日志 TX）
- mTouch 感应脚必须是 ANx（PC4 不行）

---

## qTouch IO 算法

实现以 [`qTouchIO_FT61EC2x`](qTouchIO_FT61EC2x/) 为准（`qTouchIO_FT62F21x` 为单路同源简化）。  
参数全部在 `qtouch_cfg.h`。每路一套状态，互不共用。

### 物理量

| 符号 | 含义 |
|------|------|
| S | 当前值：电荷转移次数，手指使 Cx 变大 → **S 下降** |
| B | 基线：空闲环境参考 |
| D | `max(B − S, 0)`，只认“S 低于 B”为触摸方向 |
| N | 噪声底：空闲小抖动的慢平均，0～255 |
| T | 本路按下阈值 `THRESH_MIN + N×3` |
| P | 0 松开 / 1 按下 |

`THRESH_MIN = THRESH × 65%`，释放回差 `HYST = THRESH × 40%`，CH0/CH1 各有一份。

### 电荷转移（`qtouch_acq.c`）

SNS 接铜箔，SMP 接 Cs。`DIR=0` 输出，`DIR=1` 输入。测本路前把其它路 SNS/SMP 输出低。

1. **泄放**：SNS、SMP 输出低，等 `QT_ACQ_DISCHARGE_NOPS`
2. **循环直到 SNS 读到 1 或次数到 `QT_MAX_COUNT`**
   - SMP 高阻（Cs 悬空）
   - SNS 输出高，等 `QT_CHARGE_NOPS`（给 Cx 充电）
   - SNS 改输入
   - SMP 输出低，等 `QT_TRANSFER_NOPS`（电荷 Cx→Cs）
   - 读 SNS：1 = Cs 已到约 VIH，结束；0 = 再转一次
3. 两脚再输出低

返回值即本路原始计数。当前 `QtAcq_Measure` 关中断后做 **一轮** 转移（`QT_BURST_SAMPLES` 为预留）。

`QT_IIR_SHIFT == 0`：S = 当次计数。  
`QT_IIR_SHIFT == 1`：`S = (S + 当次) / 2`。

### 空闲：跟环境

未按下且消抖计数为 0，并且 `D ≤ max(QT_NOISE_IDLE_MIN, N×3)`：

- `N = (N×7 + D) / 8`
- B：S≥B 时每轮加 `QT_BASELINE_UP_STEP`；S < B 时下漂 `>> QT_BASELINE_DOWN_SHIFT`（至少 1）

D 超出该空闲带后 **本路 B、N 冻结**，只让 S、D 变，避免把手指吃进基线。

空闲时若 `S > B + RECAL_JUMP`（遮挡拿开等正向跳变），本路重校准。

### 按下 / 松开

- **按下**：`D ≥ T` 连续 `DEBOUNCE_IN` 次 → `P=1`，B/N 保持冻结
- **松开**：`D ≤ T − HYST`（下限为本路 `THRESH_MIN`）连续 `DEBOUNCE_OUT` 次 → `P=0`
- **长按保护**：`P=1` 期间 B/N 仍冻结；扫描次数达到本路 `PRESS_TIMEOUT` 后重校准（防异物占键）

重校准：采 `CAL_SAMPLES` 次平均，写入本路 S/B，D=0，N=0，P=0。

### 扫描循环

应用与 TouchDeg 均为采完一路立刻处理：

```c
QtKey_Init();
for (;;)
{
    for (ch = 0; ch < QT_CH_COUNT; ch++)
    {
        QtKey_ScanCh(ch);
        mask = QtKey_GetPressedMask(); /* bit0=CH0，bit1=CH1 */
        /* 应用：刷新 PA7/PA6；TouchDeg：再发该路一帧 */
    }
}
```

CH0 按下 → PA7 低；CH1 按下 → PA6 低。

### 调参（IO）

| 现象 | 改什么 |
|------|--------|
| 充电不足、S 乱跳 | 加大 `QT_CHARGE_NOPS` / `QT_TRANSFER_NOPS` |
| 一轮太慢 / S 顶到上限 | 减小 `QT_MAX_COUNT`，或检查 Cs |
| 跟手慢 | `QT_IIR_SHIFT=0`，减小消抖 |
| 空闲误触 | 加大本路 `QT_CHx_THRESH` |
| 轻触不出 | 减小本路 `QT_CHx_THRESH`（须低于实际 D） |
| 松开迟 | 减小 `RELEASE_HYST` 或 `DEBOUNCE_OUT` |
| 长按几秒后自己松开 | 加大本路 `PRESS_TIMEOUT` |
| SNR/N 不跟着 S 抖 | N 只用空闲单向 D，且 D 超过空闲带即冻结；不是 S 的峰峰值 |

---

## 调试（TouchDeg）

`TouchDeg.prj`，PA2，约 9600 8N1。每个通道单独一帧（12 字节）：

```text
55 | ch | S_L S_H  B_L B_H  D_L D_H  N  P | AA
```

```text
55  00  S0l S0h  B0l B0h  D0l D0h  N0  P0  AA   ← CH0（PA7）
55  01  S1l S1h  B1l B1h  D1l D1h  N1  P1  AA   ← CH1（PA6）
```

| 字段 | 长度 | 含义 |
|------|------|------|
| `55` / `AA` | 1 | 起始 / 结束 |
| `ch` | 1 | 通道号，0=CH0，1=CH1 |
| `S` | 2 小端 | 该路滤波后信号，触摸后下降 |
| `B` | 2 小端 | 该路动态基线 |
| `D` | 2 小端 | 该路 `max(B − S, 0)` |
| `N` | 1 | 该路噪声底 |
| `P` | 1 | 该路是否按下 |

空闲该路 `S≈B`、`D` 很小；按下该路 `D` 变大、`P=1`。  
mTouch 的 `MT_LP_ENABLE` 在 TouchDeg 里不要开。

---

## 调参顺序

参数全部在各方案的 `qtouch_cfg.h` / `mtouch_cfg.h`。qTouch IO 见上文「qTouch IO 算法」。

1. TouchDeg 确认空闲 `S≈B`，按下 `D` 变大、`P=1`
2. QTouch ADC：先调 `QT_TRANSFER_PULSES`，让空闲 ADC 落在量程中下部
3. ADC / mTouch：调 `QT_TOUCH_THRESH_MIN` / `MT_TOUCH_THRESH_MIN`；IO：调本路 `QT_CHx_THRESH`
4. 仍抖：加大 `DEBOUNCE_*` / `RELEASE_HYST`
5. 环境跟不上：略增 `BASELINE_UP_STEP` 或调整 `BASELINE_DOWN_SHIFT`
6. 多路串扰：`MT_GUARD_ENABLE=1`；或拉开键距、加大阈值

mTouch 应用工程可开 WDT+SLEEP：空闲约 128 ms 唤醒扫描，按下改用 5 ms 跟手。

---

## 构建

使用 **FMD IDE** 打开各目录下的 `main.prj`（应用）或 `TouchDeg.prj`（调试）。  
时钟默认内部约 8 MHz（`OSCCON`）。

清理编译中间文件可运行仓库根目录的 `clearVxx.bat`。
