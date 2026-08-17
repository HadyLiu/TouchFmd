# ADC QTouch 触摸方案说明（FT61EC2x）

硬件：SNS + SMP + 外接 Cs，电荷从 Cx 转到 Cs。  
固定少量转移后用 **10 位 ADC** 读 Cs 电压。触摸后 adc 升、`raw = 1023 - adc` 下降。

两路；SNS 必须是 ANx（手册 AN5=PC1、AN6=PC2，PC4 无 ADC）。

---

## 1. 硬件连接

引脚在 `qtouch_cfg.h` 通道表里改，不必动 HAL：

```c
#define QT_CH_COUNT 2u

#define QT_CH0_SNS_BIT 2u /* PC2 */
#define QT_CH0_SMP_BIT 3u /* PC3 */
#define QT_CH0_SNS_AN  6u /* AN6 */

#define QT_CH1_SNS_BIT 1u /* PC1 */
#define QT_CH1_SMP_BIT 4u /* PC4 */
#define QT_CH1_SNS_AN  5u /* AN5 */
```

| 通道 | SNS | SMP |
|------|-----|-----|
| CH0 | PC2 / AN6 | PC3 |
| CH1 | PC1 / AN5 | PC4 |

指示：PA7=通道1、PA6=通道2（低有效）。日志 TX：PA2。Cs 建议 NPO/C0G；Rs 约 1kΩ。

---

## 2. 工程

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 扫描两路，PA7/PA6 低有效指示 |
| `TouchDeg.prj` | `touchdeg_main.c` | 每通道 `K n S= B= D= P=`，末行 `M=` |

Device = **FT61EC2X**。ADC：右对齐 10 位，FOSC/64，参考 VDD。

空闲 ADC 太小 → 加大 `QT_TRANSFER_PULSES`；接近 1023 饱和 → 减小次数。
