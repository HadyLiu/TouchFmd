# TouchFmd

面向 **FMD（复旦微）8 位 MCU** 的电容触摸固件：电荷转移 QTouch 与 ADC CVD 自电容。  
目标是可商用、抗干扰、RAM 占用低，引脚与算法参数全部宏定义，便于移植。

按键层共用同一套判决逻辑：中值 + IIR、噪声底自适应阈值、动态基线、迟滞消抖、上电/跳变/粘键重校准。  
触摸后信号 **S 下降**，按下判据为 `D = max(B − S, 0)`。

许可证：[Apache License 2.0](LICENSE)

---

## 方案一览

| 目录 | 芯片 | 采集原理 | 外接 Cs | 通道数 | 适用 |
|------|------|----------|---------|--------|------|
| [`qTouchIO_FT62F21x`](qTouchIO_FT62F21x/) | FT62F21x | GPIO 电荷转移，计次数到约 1/2 VDD | 要（约 22 nF） | 1 | 无 ADC 或资源极紧 |
| [`qTouchIO_FT61FC3x`](qTouchIO_FT61FC3x/) | FT61FC3x | 同上 | 要 | 1 | IO 方案移植到 FT61 |
| [`qTouchADC_FT61FC3x`](qTouchADC_FT61FC3x/) | FT61FC3x | 固定少量转移后 12 位 ADC 读 Cs | 要 | 1 | 与 IO 同硬件，次数少、分辨高 |
| [`mTouchADC_FT61FC3x`](mTouchADC_FT61FC3x/) | FT61FC3x | 片内 CVD 电容分压 | **不要** | 1～8，通道表可配 | 多路按键、低功耗 |

选型建议：

1. **多路、少外围** → `mTouchADC_FT61FC3x`
2. **单路、已有 SNS+SMP+Cs 板** → `qTouchADC_FT61FC3x`（有 ADC）或 `qTouchIO_*`（纯 GPIO）
3. **FT62F21x** → 只用 `qTouchIO_FT62F21x`

各方案细节见对应目录下的 `reademe.md`。

---

## 仓库结构

```text
TouchFmd/
├── qTouchIO_FT62F21x/     # GPIO QTouch，FT62F21x
├── qTouchIO_FT61FC3x/     # GPIO QTouch，FT61FC3x
├── qTouchADC_FT61FC3x/    # ADC QTouch，FT61FC3x
├── mTouchADC_FT61FC3x/    # 多路 ADC CVD，FT61FC3x
├── clearVxx.bat           # 清理 IDE 中间文件
└── LICENSE
```

每个方案都是独立工程，典型文件：

| 文件 | 职责 |
|------|------|
| `*_cfg.h` | 引脚、通道、全部算法参数 |
| `*_hal.c/h` | GPIO / ADC 寄存器操作 |
| `*_acq.c/h` | 单次采集 + 突发去极值平均 |
| `*_key.c/h` | 滤波、基线、判决、校准 |
| `main.c` | 应用入口：扫描触摸，按下时 PA1 输出高 |
| `touchdeg_main.c` | 调试入口：PA1 软串口约 9600 上报 |
| `soft_uart.c/h` | TouchDeg 软串口（无库除法打数字） |
| `main.prj` / `TouchDeg.prj` | FMD IDE 工程 |

---

## 硬件要点

### QTouch（IO / ADC）

SNS + SMP + 外接 Cs。手指靠近 → Cx 变大 → 每次转移电荷更多 → **S 下降**。

```text
  [铜箔 Cx]──[Rs ≈ 1 kΩ]──┬── GPIO SNS（默认 PA0）
                          │
                         Cs ≈ 22 nF NPO/C0G
                          │
                          └── GPIO SMP（默认 PA4）
```

| 网络 | 默认脚 | 说明 |
|------|--------|------|
| SNS | PA0 | 感应脚；ADC 方案兼 AN0 |
| SMP | PA4 | 采样脚，接 Cs 另一端 |
| Deg / 指示 | PA1 | TouchDeg TX；应用按下指示 |

ADC 方案与 IO **硬件相同**：IO 要转到数字口约 1/2 VDD（可达数百次）；ADC 只固定转移少量次数（默认 64）再读 Cs 电压。

### mTouch CVD

每路铜箔经 Rs 接到对应 `ANx`（`ANx = PAx`）。片内 C_hold 与 Cx 分压，**无外接 22 nF**。  
另需一颗 **PRE dummy**（默认 PA2），不接铜箔，用于预充 C_hold。

默认 3 路：PA0 / PA3 / PA4。改通道只动 `mtouch_cfg.h` 的 `MT_CH_COUNT` 与 `MT_CH_ADC_INIT`。

### 公共约束

- Cs 用 NPO/C0G；Rs 约 1 kΩ
- 测量时关闭感应脚上拉
- **勿用 PA6**（ISP CLK）
- 勿占用 PA1（指示 / 软串口）
- mTouch 的 PRE 不要出现在通道表里，也不要接到铜箔

---

## 按键算法（共用）

每轮扫描顺序：

1. **突发采集**：关中断，采 6 点，去最大最小，剩余 4 点 `>>2` 平均
2. **三点中值** → **一阶 IIR** `filt = (filt*3 + med)/4` → 得到 S
3. **差值** `D = max(B − S, 0)`；阈值 `T = THRESH_MIN + noise×3`
4. **噪声底**：仅空闲且未过阈值时用残差 IIR 更新
5. **动态基线**：未按快上慢下，按下冻结
6. **迟滞 + 进出消抖** 确认按下 / 释放
7. **重校准**：上电平均、正向跳变、粘键超时

热路径避免库乘除（移位或 `(n<<1)+n`），适配 FT62 等 RAM/指令紧的芯片。

应用侧典型用法：

```c
QtKey_Init();           /* 或 MtKey_Init() */

while (1)
{
    QtKey_Scan();
    st = QtKey_GetStatus();
    App_SetOut(st->pressed);   /* 1 = 按下 */
    DelayMs(5);
}
```

多路 mTouch 用 `MtKey_GetPressedMask()`：bit0 = 通道 0，bit1 = 通道 1，…  
任意键按下时默认 PA1 输出高。

---

## 调试（TouchDeg）

用 FMD IDE 打开对应目录的 `TouchDeg.prj`，PA1 软串口约 **9600** 上报。

单路：

```text
S=<signal> B=<baseline> D=<delta> N=<noise> T=<thresh> P=<pressed>
```

多路每通道一行 `K n S= B= D= P=`，末行 `M=` 为按下掩码。

空闲应 `S≈B`、`D` 很小；按下该路 `D` 明显变大、`P=1`。  
**TouchDeg 不要开低功耗**（mTouch 的 `MT_LP_ENABLE`），否则串口会丢。

---

## 调参顺序

参数全部在各方案的 `qtouch_cfg.h` / `mtouch_cfg.h`。

1. TouchDeg 确认空闲 `S≈B`，按下 `D` 变大
2. QTouch ADC：先调 `QT_TRANSFER_PULSES`，让空闲 ADC 落在量程中下部（太小加大次数，接近 4095 则减小或检查 Cs）
3. 再调 `QT_TOUCH_THRESH_MIN` / `MT_TOUCH_THRESH_MIN`：空闲不误触、轻触能触发
4. 仍抖：加大 `DEBOUNCE_*` / `RELEASE_HYST`
5. 环境跟不上：略增 `BASELINE_UP_STEP` 或调整 `BASELINE_DOWN_SHIFT`
6. 多路串扰：`MT_GUARD_ENABLE=1` 邻键拉低屏蔽；或拉开键距、加大阈值
7. 粘键过早松开感：增大 `STUCK_LIMIT`（`uchar`，≤255）

mTouch 应用工程可开 WDT+SLEEP：空闲约 128 ms 唤醒扫描，按下改用 5 ms 跟手。粘键时间约为 `STUCK_LIMIT × 扫描周期`。

---

## 构建

使用 **FMD IDE** 打开各目录下的 `main.prj`（应用）或 `TouchDeg.prj`（调试）。  
时钟默认内部约 8 MHz（`OSCCON`）。

清理编译中间文件可运行仓库根目录的 `clearVxx.bat`。
