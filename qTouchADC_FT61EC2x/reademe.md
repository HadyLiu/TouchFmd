# ADC QTouch 触摸方案说明（FT61EC2x）

硬件：SNS + SMP + 外接 Cs。固定 `QT_TRANSFER_PULSES` 次转移后，10 位 ADC 读 Cs。  
触摸后 adc 升高，`S = 1023 − adc` **下降**。按键层与 IO 方案相同，见仓库根目录 `readme.md`「qTouch IO 算法」。

SNS 必须是 ANx（AN5=PC1、AN6=PC2，PC4 无 ADC，只作 SMP）。

---

## 1. 硬件连接

引脚在 `qtouch_cfg.h` 通道表里改：

| 通道 | SNS | SMP |
|------|-----|-----|
| CH0 | PC2 / AN6 | PC3 |
| CH1 | PC1 / AN5 | PC4 |

指示：PA7=CH0、PA6=CH1（低有效）。日志 TX：PA2。Cs 用 NPO/C0G；Rs 约 1 kΩ。上电 `WPUC=0`。

---

## 2. 工程

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 逐路 `QtKey_ScanCh`，PA7/PA6 低有效指示 |
| `TouchDeg.prj` | `touchdeg_main.c` | 先扫 8 轮刷新灯，再发 12 字节帧 `55 ch S B D SNR P AA` |

Device = **FT61EC2X**。ADC：右对齐 10 位，FOSC/64，参考 VDD。

空闲 ADC 太小 → 加大 `QT_TRANSFER_PULSES`；接近 1023 饱和 → 减小次数。  
检测灵敏度调本路 `QT_CHx_THRESH`（固定 `T = THRESH×65%`，N 只表示空闲抖动、不抬阈值）。
