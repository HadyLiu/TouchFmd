# mTouch ADC CVD 多路触摸（FT61EC2x）

片内 **10 位 ADC** + **CVD（电容分压）** 自电容。  
**两路**，不需要外接 Cs。C_hold 用内部 **1/4VDD (AN7)** 预充，不占 GPIO。  
按键算法与 QTouch 相同（中值、IIR、噪声底、动态基线、迟滞消抖、校准）。

---

## 1. 通道（`mtouch_cfg.h`）

只改通道表宏，INIT 数组会自动展开：

```c
#define MT_CH_COUNT 2u          /* 单路改成 1u 即可 */

#define MT_CH0_ADC 6u           /* AN6 */
#define MT_CH0_BIT 2u           /* PC2 */

#define MT_CH1_ADC 5u           /* AN5 */
#define MT_CH1_BIT 1u           /* PC1 */

#define MT_PRE_ADC 7u           /* 内部 1/4VDD */
```

| 项 | 含义 |
|----|------|
| `MT_CH_COUNT` | 按键路数，1 或 2 |
| `MT_CHn_ADC` | 该路 ADC 通道号（CHS） |
| `MT_CHn_BIT` | 该路 PORTC 位号（ANx ≠ PCx 位号） |
| `MT_PRE_ADC` | 固定 7=内部 1/4VDD；须 `ANSEL[7]=1` |

手册映射：AN0=PA0 … AN3=PA3，AN4=PC0，**AN5=PC1**，**AN6=PC2**，AN7=1/4VDD。  
**PC4 无 ADC**，不能作感应脚。

**不要占用：** PA7/PA6（指示）、PA2（日志 TX）。PA0 为 ISP CLK。

---

## 2. 硬件

```
  [铜箔 K0]──[Rs]── PC2 / AN6
  [铜箔 K1]──[Rs]── PC1 / AN5
                    内部 AN7      PRE = 1/4VDD
                    PA7           通道1 指示（低有效）
                    PA6           通道2 指示（低有效）
                    PA2           TouchDeg TX
```

Rs 约 1kΩ。无外接 22nF。测量时关上拉。

`MT_GUARD_ENABLE=1`：测量时把其它感应脚拉低作地屏蔽。

通道1 按下 → PA7 低；通道2 按下 → PA6 低。`MtKey_GetPressedMask()` bit0=K0，bit1=K1。

---

## 3. CVD 原理

片内 **C_hold** 先接到 AN7 充到 **1/4VDD**，再与电极 **Cx** 分压。

| 极性 | 预充 | 电极 | 结果 |
|------|------|------|------|
| A | C_hold=1/4VDD | Cx=0 | 触摸后 Va 下降 |
| B | C_hold=1/4VDD | Cx=VDD | 触摸后 Vb 上升 |

```text
raw = Va + (1023 - Vb)    // 触摸后下降
```

ADC：右对齐 10 位，参考 VDD，时钟 FOSC/64（与官网 ADC demo 一致）。突发 6 点去极值后 `>>2`。

---

## 4. 工程

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 扫描全部通道，PA7/PA6 低有效指示 |
| `touchDeg.prj` | `touchdeg_main.c` | 每通道 `K n S= B= D= P=`，末行 `M=` 掩码 |

Device = **FT61EC2X**。SRAM 128 字节，两路按键状态约 24 字节。

---

## 5. 低功耗（main.prj）

空闲：关 ADC → `SLEEP()`，WDT 约 128ms 唤醒再扫。  
按下：`DelayMs(5)` 跟手。TouchDeg 不要开低功耗。

---

## 6. 调参

1. TouchDeg：空闲 `S≈B`；按下该路 `D` 变大、`P=1`
2. 先调 `MT_TOUCH_THRESH_MIN`
3. 串扰：`MT_GUARD_ENABLE=1`
