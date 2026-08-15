# mTouch ADC CVD 多路触摸（FT61FC3x）

片内 12 位 ADC + **CVD（电容分压）** 自电容。  
**多路、通道表可自定义**，不需要外接 Cs。按键算法与 QTouch 相同（中值、IIR、噪声底、动态基线、迟滞消抖、校准）。

---

## 1. 自定义通道

只改 `mtouch_cfg.h`：

```c
#define MT_CH_COUNT     3u
#define MT_CH_ADC_INIT  { 0u, 3u, 4u }   /* ANx = PAx */
#define MT_PRE_ADC      2u               /* dummy，不接铜箔 */
```

| 项 | 含义 |
|----|------|
| `MT_CH_COUNT` | 按键路数，1～8 |
| `MT_CH_ADC_INIT` | 每路 ADC 通道号，**必须与 COUNT 个数一致** |
| `MT_PRE_ADC` | C_hold 预充脚，**不要出现在通道表里** |

FT61FC3x：`ANx = PAx`（AN0=PA0 … AN7=PA7），模拟口走 `ANSEL0`。

**不要占用：** PA1（指示/软串口）、PA6（ISP CLK）、`MT_PRE_ADC`。

四路示例（PA0/PA3/PA4/PA5）：

```c
#define MT_CH_COUNT     4u
#define MT_CH_ADC_INIT  { 0u, 3u, 4u, 5u }
```

单路：

```c
#define MT_CH_COUNT     1u
#define MT_CH_ADC_INIT  { 0u }
```

---

## 2. 硬件

```
  [铜箔 K0]──[Rs]── PA0 / AN0     通道表第 0 路
  [铜箔 K1]──[Rs]── PA3 / AN3     第 1 路
  [铜箔 K2]──[Rs]── PA4 / AN4     第 2 路
                    PA2 / AN2     PRE dummy，PCB 悬空
                    PA1           指示 / TouchDeg TX
```

Rs 约 1kΩ。无外接 22nF。测量时关上拉。

`MT_GUARD_ENABLE`（`mtouch_cfg.h`）：**1** 测量时把其它感应脚拉低作地屏蔽；**0** 其它脚改为模拟高阻，不屏蔽。

默认：任意键按下 → PA1 高。`MtKey_GetPressedMask()` bit0=K0，bit1=K1，…

---

## 3. CVD 原理

片内 **C_hold** 与电极 **Cx** 分压。手指靠近 → Cx 变大。

| 极性 | 预充 | 结果 |
|------|------|------|
| A | C_hold=VDD，Cx=0 | `Va = VDD·Chold/(Chold+Cx)`，触摸后下降 |
| B | C_hold=0，Cx=VDD | `Vb = VDD·Cx/(Chold+Cx)`，触摸后上升 |

```text
raw = Va + (4095 - Vb)    // 触摸后下降，与 qtouch_key 同向
```

ADC：右对齐 12 位，参考 VDD/GND。突发 6 点去极值后 `>>2`。

---

## 4. 工程

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 扫描全部通道，任意按下 PA1 高 |
| `touchDeg.prj` | `touchdeg_main.c` | 每通道上报 `K n S= B= D= P=`，末行 `M=` 掩码 |

官网 `FT61FC3x_ADC.C` 仅作 ADC 寄存器参考，不参与编译。

SRAM：每通道约 12 字节，默认 3 路约 36 字节（芯片 256B）。

---

## 5. 低功耗（main.prj）

`mtouch_cfg.h`：

```c
#define MT_LP_ENABLE            1u   /* 1=开 WDT+SLEEP；0=DelayMs 忙等 */
#define MT_LP_IDLE_ONLY         1u   /* 1=仅空闲休眠，按下用 5ms 跟手 */
#define MT_LP_WDTCON            0B00001111  /* ≈128ms 唤醒 */
```

空闲：关 ADC → `SLEEP()`，WDT 约 128ms 唤醒再扫。  
按下：不睡，`DelayMs(5)` 扫描，释放后回到休眠。  
TouchDeg 不要开低功耗（串口会丢）。

休眠后扫描周期变长，`MT_STUCK_LIMIT` 对应时间约 `LIMIT × 128ms`。

---

## 6. 调参

1. TouchDeg：空闲 `S≈B`、`D` 很小；按下该路 `D` 变大、`P=1`  
2. 先调 `MT_TOUCH_THRESH_MIN`  
3. 串扰：`MT_GUARD_ENABLE=1` 邻键拉低屏蔽；关 Guard 时拉开键距或加大阈值  
4. PRE 不要接到铜箔或按键
