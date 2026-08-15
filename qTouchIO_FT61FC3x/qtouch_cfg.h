#ifndef QTOUCH_CFG_H
#define QTOUCH_CFG_H

#include "SYSCFG.h"

/*=================================================
 * qtouch_cfg.h — FT61FC3x 移植版
 *
 * 相对 FT62F21x：算法参数可先保持不变；芯片差异在
 * ANSEL0（数字口）、无 PSRCA/PSINKA、以及 PA6=ISP CLK。
 *
 * 调参顺序建议（TouchDeg 看 S/B/D/N/T/P）：
 * 1) 先确认空闲时 S≈B、D 很小；按下时 D 明显变大
 * 2) 调 QT_TOUCH_THRESH_MIN，使空闲不误触、轻触能触发
 * 3) 噪声大再加大滤波 / BURST；误释放调 HYST、DEBOUNCE
 *=================================================*/

/*-------------------------------------------------
 * 引脚（移植只改 BIT；PORT/TRIS/WPU/ANSEL 跟芯片头文件）
 * SNS：感应脚，接铜箔侧（经 Rs）
 * SMP：采样脚，接 Cs 另一端
 * FT61FC3x SOP20：勿用 PA6（ISP CLK）
 --------------------------------------------------*/
#define QT_SNS_PORT PORTA
#define QT_SNS_TRIS TRISA
#define QT_SNS_WPU WPUA
#define QT_SNS_BIT 0u /* SNS = PA0 */

#define QT_SMP_PORT PORTA
#define QT_SMP_TRIS TRISA
#define QT_SMP_WPU WPUA
#define QT_SMP_BIT 4u /* SMP = PA4 */

#define QT_APP_OUT_BIT 1u     /* 应用工程：按下指示脚 PA1 */
#define QT_DBG_TX_BIT 1u      /* TouchDeg：软串口 TX = PA1 */
#define QT_DBG_BAUD_DELAY 33u /* 软串口位延时；↑变慢 ↓变快，对 9600 */

/*-------------------------------------------------
 * 采集（电荷转移）
 * QT_MAX_COUNT        单次最大转移次数上限；越大动态范围越大，耗时越长
 * QT_CHARGE_NOPS      Cx 充电稳定 NOP 数；偏小充电不足，偏大变慢
 * QT_TRANSFER_NOPS    Cx→Cs 转移稳定 NOP 数
 * QT_ACQ_DISCHARGE_NOPS  测量前泄放 Cs 的 NOP 数
 * QT_BURST_SAMPLES    突发采样次数（须为 6：去掉最大最小后剩 4）
 * QT_BURST_AVG_SHIFT  对剩余 4 点做 >>2 平均（必须=2，与上面配套）
 --------------------------------------------------*/
#define QT_MAX_COUNT 600u
#define QT_CHARGE_NOPS 3u
#define QT_TRANSFER_NOPS 3u
#define QT_BURST_SAMPLES 6u
#define QT_BURST_AVG_SHIFT 2u
#define QT_ACQ_DISCHARGE_NOPS 6u

/*-------------------------------------------------
 * 滤波（值越大越平滑、响应越慢）
 * QT_IIR_SHIFT        信号 IIR：filt=(filt*3+x)/4 → shift=2
 * QT_NOISE_SHIFT      噪声底 IIR：/8 → shift=3
 --------------------------------------------------*/
#define QT_IIR_SHIFT 2u
#define QT_NOISE_SHIFT 3u

/*-------------------------------------------------
 * 判决 / 去抖
 * 实际按下阈值 T = QT_TOUCH_THRESH_MIN + noise*3
 *   noise 由空闲残差自动估计；干扰大时 T 自动抬高
 * QT_TOUCH_THRESH_MIN 最小阈值；太小易误触，太大要用力按
 * QT_RELEASE_HYST     释放回差：释放阈值 ≈ T - HYST，防临界抖动
 * QT_DEBOUNCE_IN      连续满足按下条件的次数才确认按下
 * QT_DEBOUNCE_OUT     连续满足释放条件的次数才确认释放
 --------------------------------------------------*/
#define QT_TOUCH_THRESH_MIN 10u
#define QT_RELEASE_HYST 5u
#define QT_DEBOUNCE_IN 3u
#define QT_DEBOUNCE_OUT 3u

/*-------------------------------------------------
 * 动态基线（未按下才跟踪；按下冻结）
 * QT_BASELINE_UP_STEP     信号高于基线时，每扫描跟上的步长（快跟环境）
 * QT_BASELINE_DOWN_SHIFT  信号低于基线且未过阈值时，下漂量 >>5（/32，慢跟）
 *                         慢跟是为了避免把真触摸慢慢“吃”进基线
 --------------------------------------------------*/
#define QT_BASELINE_UP_STEP 1u
#define QT_BASELINE_DOWN_SHIFT 5u

/*-------------------------------------------------
 * 自动校准 / 重校准
 * QT_CAL_SAMPLES / QT_CAL_SHIFT  上电与重校准采样次数；8 点 → >>3 平均
 * QT_RECAL_JUMP   信号比基线突然高出该值（如拔插、强干扰后）→ 强制重校准
 * QT_STUCK_LIMIT  连续判定按下超过该扫描次数视为粘键 → 重校准（uchar≤255）
 *                 约 LIMIT × 扫描周期；main 里 DelayMs(5) 时 200≈1s
 --------------------------------------------------*/
#define QT_CAL_SAMPLES 8u
#define QT_CAL_SHIFT 3u
#define QT_RECAL_JUMP 40u
#define QT_STUCK_LIMIT 200u

#endif
