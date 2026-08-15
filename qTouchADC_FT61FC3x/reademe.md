# ADC QTouch 触摸方案说明（FT61FC3x）

硬件与 IO 方案相同：SNS + SMP + 外接 Cs，电荷从 Cx 转到 Cs。  
差别：IO 要一直转到数字口读到约 1/2 VDD（常数百次）；本方案只做固定少量转移，再用 12 位 ADC 读 Cs 电压，**次数少、分辨高**。

按键算法共用 `qtouch_key`（触摸后 S 下降）。

---

## 1. 硬件连接

```
                                                   MCU
                                      ┌──────────────────────────┐
  [ 触摸按键铜箔 ]───[ Rs: 1kΩ ]───┐  │                          │
  (PCB 铜片 Cx)                   ├──┤ GPIO/ADC SNS (PA0/AN0)   │
                                 │  │                          │
                             ┌───┴───┐│                          │
                             │  Cs   ││                          │
                             │ 22nF  ││                          │
                             │ (NPO) ││                          │
                             └───┬───┘│                          │
                                 │    │                          │
                                 └────┤ GPIO_SMP (PA4)           │
                                      │                          │
                                      └──────────────────────────┘
```

| 网络 | 默认脚 | 说明 |
|------|--------|------|
| SNS | PA0 / AN0 | 感应脚，经 Rs 接铜箔；转移后 ADC 读 Cs |
| SMP | PA4 | 采样脚，接 Cs 另一端（纯 GPIO） |
| Deg / 指示 | PA1 | TouchDeg 软串口 TX；应用按下指示 |

Cs 建议 NPO/C0G；Rs 约 1kΩ。测量时关上拉。勿用 PA6（ISP CLK）。

---

## 2. 工程划分

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 应用：扫描触摸，按下时 PA1 输出高 |
| `TouchDeg.prj` | `touchdeg_main.c` | 调试：PA1 软串口约 9600 上报 |

```text
S=<signal> B=<baseline> D=<delta> N=<noise> T=<thresh> P=<pressed>
```

触摸后 **S 下降**，**D = max(B−S, 0)** 变大。

---

## 3. 采集逻辑

与 IO 相同的充/转时序，只循环 `QT_TRANSFER_PULSES` 次（默认 64，IO 常要数到 600）：

```
① SNS=OUT0, SMP=OUT0          泄放 Cs
② 重复 N 次：
     SMP=INPUT, SNS=OUT1      给 Cx 充电
     SNS=INPUT, SMP=OUT0      电荷转入 Cs
③ SNS 改为模拟，ADC 读 Cs 电压（SMP 保持低）
④ raw = 4095 - adc            触摸后 adc 升、raw 降
⑤ SNS/SMP 拉低泄放
```

突发：关中断、6 点去最大最小、`>>2` 平均。

空闲 ADC 太小（D 也小）→ 加大 `QT_TRANSFER_PULSES`。  
ADC 接近 4095（饱和）→ 减小次数或检查 Cs。

---

## 4. 与 IO 方案对照

| | qTouchIO | qTouchADC |
|--|----------|-----------|
| 引脚 | SNS + SMP | **相同** |
| 外接 Cs | 要 | **要** |
| 判据 | 数字口 VIH，计次数 | 固定 N 次后 ADC 读电压 |
| 单次循环 | 直到 ~1/2 VDD（可达 600） | 默认 64 次 + 1 次 ADC |
| 触摸后 S | 下降 | 下降（`4095-adc`） |
| 按键层 | `qtouch_key` | 相同 |

---

## 5. 调参

1. TouchDeg：空闲 `S≈B`，按下 `D` 明显变大  
2. 先调 `QT_TRANSFER_PULSES`，让空闲 ADC 大约在量程中下部  
3. 再调 `QT_TOUCH_THRESH_MIN`  
4. 仍抖：加大消抖 / `RELEASE_HYST`
