# IO QTouch 触摸方案说明

利用普通 GPIO 实现电荷转移式电容触摸（QTouch / Charge Transfer）。  
目标：可商用、加强抗干扰、可移植；引脚与参数全部宏定义（`qtouch_cfg.h`）。

目标芯片示例：FT62F21X（资源紧，算法已按少 RAM、少乘除优化）。

---

## 1. 硬件连接

```
                                                   MCU
                                      ┌──────────────────────────┐
                                      │                          │
  [ 触摸按键铜箔 ]───[ Rs: 1kΩ ]───┐  │                          │
  (PCB 铜片 Cx)                   ├──┤ GPIO_SNS (PA0)           │
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
| SNS | PA0 | 感应脚，接铜箔侧（经 Rs） |
| SMP | PA4 | 采样脚，接 Cs 另一端 |
| Deg / 应用指示 | PA1 | TouchDeg 软串口 TX；应用工程按下指示 |

Cs 建议 NPO/C0G；Rs 约 1kΩ。SNS/SMP 测量时必须关闭上拉。

---

## 2. 工程划分

| 工程 | 入口 | 作用 |
|------|------|------|
| `main.prj` | `main.c` | 应用：扫描触摸，按下时 PA1 输出高 |
| `TouchDeg.prj` | `touchdeg_main.c` | 调试：PA1 软串口约 9600 上报数据 |

TouchDeg 输出格式：

```text
S=<signal> B=<baseline> D=<delta> N=<noise> T=<thresh> P=<pressed>
```

- **S**：滤波后计数值（触摸后通常下降）
- **B**：动态基线
- **D**：`max(B-S, 0)`，按下判据
- **N**：噪声底估计
- **T**：当前自适应阈值 `THRESH_MIN + N*3`
- **P**：0 释放 / 1 按下

---

## 3. 文件结构

| 文件 | 职责 |
|------|------|
| `qtouch_cfg.h` | 引脚宏、全部算法参数 |
| `qtouch_hal.c/h` | SNS/SMP 推挽高/低、高阻输入、读电平 |
| `qtouch_acq.c/h` | 单次电荷转移 + 突发去极值平均 |
| `qtouch_key.c/h` | 中值、IIR、噪声、基线、判决、校准 |
| `delay.c/h` | 延时 |
| `soft_uart.c/h` | TouchDeg 软串口（无库除法打数字） |
| `main.c` | 应用入口 |
| `touchdeg_main.c` | 调试入口 |

---

## 4. 物理原理与采集逻辑（qtouch_acq）

### 4.1 基本思想

1. 先给电极电容 **Cx** 充到已知电位  
2. 再把电荷转移到采样电容 **Cs**  
3. 用数字口读 SNS，判断 Cs 电压是否到达约 **1/2 VCC**（CMOS VIH）  
4. 重复“充电 Cx → 转移 → 判断”，直到 SNS 读到高，累计转移次数为 **raw count**

手指靠近 → Cx 变大 → 每次转移电荷更多 → **更少次数**即可到阈值 → **计数值下降**。

### 4.2 单次测量 `QtAcq_MeasureOnce`

```
① SNS=OUT0, SMP=OUT0          泄放 Cs（及通路）
② SMP=INPUT(高阻), SNS=OUT1   给 Cx 充电（Cs 两端近似同电位变化，几乎不充 Cs）
③ SNS=INPUT(高阻), SMP=OUT0   电荷从 Cx 转入 Cs
④ 读 SNS：若为高 → 结束；否则 count++，回到 ②
⑤ 结束时 SNS/SMP 拉低泄放
```

- `count` 上限：`QT_MAX_COUNT`  
- 充/转/泄放稳定：`QT_CHARGE_NOPS` / `QT_TRANSFER_NOPS` / `QT_ACQ_DISCHARGE_NOPS`

### 4.3 突发测量 `QtAcq_Measure`（提 SNR）

1. 保存并关闭中断（`INTCON=0`），保证时序一致  
2. 连续采 `QT_BURST_SAMPLES`（6）次 `MeasureOnce`  
3. 去掉 **1 个最大 + 1 个最小**  
4. 剩余 4 点求和后 `>> QT_BURST_AVG_SHIFT`（`/4`，无除法指令）  
5. 恢复中断  

返回值作为本轮原始信号 `raw`。

---

## 5. 按键算法全流程（qtouch_key）

每轮 `QtKey_Scan()` 顺序如下。

### 5.1 多重数字滤波

**① 三点滑动中值**

- 窗口：`s_med1, s_med2, raw`  
- 取中值 `med`，抑制尖峰干扰  

**② 一阶 IIR 低通**

```text
filt = (filt * 3 + med) / 4
     = ((filt << 2) - filt + med) >> 2    // QT_IIR_SHIFT=2
```

`s_status.signal = filt`（即上报的 S）。

### 5.2 差值与自适应阈值

```text
delta = max(baseline - filt, 0)          // 上报 D
thresh T = QT_TOUCH_THRESH_MIN + noise*3
         = MIN + (noise<<1) + noise      // 无乘指令
```

**噪声底 `noise`（上报 N）**

- 仅在 **未按下** 且 `delta < T` 时更新  
- 用相对基线的残差做 IIR：

```text
noise = (noise * 7 + residual) / 8
      = ((noise << 3) - noise + residual) >> 3
```

干扰大 → N 升 → T 自动抬高 → 降低误触。

### 5.3 动态自适应基线跟踪

- **按下期间**：基线 **冻结**（不跟踪）  
- **未按下**：

| 情况 | 行为 |
|------|------|
| `signal >= baseline` | 每扫描 `+ QT_BASELINE_UP_STEP`（上漂快跟环境/电源） |
| `signal < baseline` 且 `delta < T` | 下漂量 `delta >> QT_BASELINE_DOWN_SHIFT`（默认 /32，慢跟） |
| `delta >= T` | 不把这段下漂吃进基线（防吞真触摸） |

### 5.4 判决、迟滞与去抖

**未按下 → 按下**

- 条件：`delta >= T`  
- 连续满足 `QT_DEBOUNCE_IN` 次 → `pressed = 1`  

**按下 → 释放**

- 释放阈值：`release_thr = T - QT_RELEASE_HYST`（若不够则退回 `THRESH_MIN`）  
- 条件：`delta <= release_thr`  
- 连续满足 `QT_DEBOUNCE_OUT` 次 → `pressed = 0`  

迟滞避免在阈值附近来回抖；消抖抑制短脉冲干扰。

### 5.5 自动校准与重新校准

**上电 / 显式校准 `QtKey_Recalibrate`**

1. 采 `QT_CAL_SAMPLES`（8）次 `QtAcq_Measure`  
2. 平均：`sum >> QT_CAL_SHIFT`（`/8`）  
3. 重置：`baseline = signal = 该均值`，`noise=0`，`pressed=0`，中值窗与计数清零  

`QtKey_Init` = `QtAcq_Init` + `QtKey_Recalibrate`。

**异常正向跳变重校准**

```text
若 filt > baseline + QT_RECAL_JUMP  →  Recalibrate()
```

用于强干扰、拔插、环境突变后基线失效。

**粘键超时重校准**

- 按下期间 `s_stuck++`  
- `s_stuck >= QT_STUCK_LIMIT`（默认 200，`uchar`）→ `Recalibrate()`  
- 时间约 `LIMIT × 扫描周期`（`main` 中 `DelayMs(5)` 时约 1s）  

防止导电物一直贴住导致永久“按下”。

---

## 6. 状态机概览

```text
                    ┌──────────────┐
                    │  Recalibrate │←── 上电 / 跳变 / 粘键
                    └──────┬───────┘
                           ▼
              ┌────────────────────────┐
         ┌───►│      IDLE (未按下)      │◄───┐
         │    │ 跟踪基线、更新 noise    │    │
         │    │ delta>=T 累计消抖      │    │
         │    └──────────┬─────────────┘    │
         │               │ DEBOUNCE_IN 到   │
         │               ▼                  │
         │    ┌────────────────────────┐    │
         │    │     PRESSED (按下)      │    │
         │    │ 冻结基线；stuck++       │    │
         │    │ delta<=T-HYST 累计消抖  │────┘
         │    └──────────┬─────────────┘  DEBOUNCE_OUT 到
         │               │
         └───────────────┘  (释放后回 IDLE)
```

---

## 7. HAL 逻辑（qtouch_hal）

- 初始化：SNS/SMP 关上拉，输出低  
- `SnsOutHigh/Low`、`SmpOutLow`：推挽输出  
- `SnsInput` / `SmpInput`：高阻输入（转移/充电阶段）  
- `SnsRead`：读 PORT 位判断 VIH  

移植：只改 `qtouch_cfg.h` 中 PORT/TRIS/WPU/BIT。

---

## 8. 实现约束（轻量商用）

- 热路径 **避免库乘除**：平均、IIR、噪声、阈值增益一律移位或 `(n<<1)+n`  
- RAM 紧：噪声/`stuck`/消抖用 `uchar`；中值窗只保留 2 个历史词  
- 调试串口十进制用位权减法，不调用 `/` `%` 库  

参数含义与调参顺序见 `qtouch_cfg.h` 文件头注释。

---

## 9. 调参建议

1. TouchDeg 空闲：`S≈B`，`D` 很小；按下：`D` 明显变大  
2. 先调 `QT_TOUCH_THRESH_MIN`：不误触、轻触能触发  
3. 仍抖：加大消抖 / `RELEASE_HYST`，或略增 IIR、确认突发 6 点配套  
4. 环境漂移跟不上：略增 `BASELINE_UP_STEP` 或减小 `DOWN_SHIFT`（更慢下漂要更大 shift）  
5. 粘键过早松开感：增大 `STUCK_LIMIT`（≤255）  

---

## 10. 算法清单（与代码对应）

1. **电荷转移采集**：Cx 充电 → 转 Cs → 判约 1/2VCC → 计次数  
2. **突发去极值平均**：关中断、去最大最小、移位平均  
3. **三点中值滤波**  
4. **一阶 IIR 低通**  
5. **噪声底估计 + 自适应阈值** `T = MIN + noise*3`  
6. **动态基线**：未按快上慢下，按下冻结  
7. **迟滞判决 + 进出消抖**  
8. **上电校准 / 跳变重校准 / 粘键重校准**  
