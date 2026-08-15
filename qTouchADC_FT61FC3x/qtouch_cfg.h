#ifndef QTOUCH_CFG_H
#define QTOUCH_CFG_H

#include "SYSCFG.h"

/*=================================================
 * qtouch_cfg.h — FT61FC3x ADC 电荷转移 QTouch
 *
 * 硬件与 IO 方案相同（SNS + SMP + Cs）。
 * 差别：不等数字口 VIH，只做少量转移后用 ADC 读 Cs 电压，
 * 从而减少转移次数。触摸后 Cs 电压升高，raw 取满量程减 ADC，
 * 使 S 仍下降，qtouch_key 不用改。
 *
 * 调参顺序（TouchDeg 看 S/B/D/N/T/P）：
 * 1) 空闲 S≈B、D 很小；按下 D 明显变大
 * 2) 空闲 ADC 太小则加大 QT_TRANSFER_PULSES；接近饱和则减小
 * 3) 再调 QT_TOUCH_THRESH_MIN
 *=================================================*/

/*-------------------------------------------------
 * 引脚（与 IO 方案相同）
 * SNS：感应脚，接铜箔侧（经 Rs），兼 ADC 通道
 * SMP：采样脚，接 Cs 另一端
 * FT61FC3x：勿用 PA6（ISP CLK）
 --------------------------------------------------*/
#define QT_SNS_PORT             PORTA
#define QT_SNS_TRIS             TRISA
#define QT_SNS_WPU              WPUA
#define QT_SNS_ANSEL            ANSEL0
#define QT_SNS_BIT              0u          /* SNS = PA0 / AN0 */
#define QT_SNS_ADC_CH           0u

#define QT_SMP_PORT             PORTA
#define QT_SMP_TRIS             TRISA
#define QT_SMP_WPU              WPUA
#define QT_SMP_BIT              4u          /* SMP = PA4 */

#define QT_APP_OUT_BIT          1u          /* 应用：按下指示 PA1 */
#define QT_DBG_TX_BIT           1u          /* TouchDeg：软串口 TX = PA1 */
#define QT_DBG_BAUD_DELAY       33u

/*-------------------------------------------------
 * 电荷转移 + ADC
 * IO 方案要转到 Cs≈1/2VDD（常数百次）。ADC 12 位可在更低电压
 * 读出差值，故 QT_TRANSFER_PULSES 远小于 QT_MAX_COUNT。
 *
 * QT_TRANSFER_PULSES  固定转移次数；↑电压更高、delta 更大，耗时更长
 * QT_CHARGE_NOPS      Cx 充电稳定 NOP
 * QT_TRANSFER_NOPS    Cx→Cs 转移稳定 NOP
 * QT_ACQ_DISCHARGE_NOPS  测量前泄放 Cs
 * QT_ADC_TACQ_NOPS    切到 SNS 通道后采样保持
 * QT_ADC_FULLSCALE    12 位满量程；raw = FULLSCALE - adc
 * QT_ADC_GO_TIMEOUT   等待 GO/DONE 循环上限
 --------------------------------------------------*/
#define QT_TRANSFER_PULSES      64u
#define QT_CHARGE_NOPS          3u
#define QT_TRANSFER_NOPS        3u
#define QT_ACQ_DISCHARGE_NOPS   6u
#define QT_ADC_TACQ_NOPS        8u
#define QT_ADC_FULLSCALE        4095u
#define QT_ADC_GO_TIMEOUT       2000u

/*-------------------------------------------------
 * 突发采样（须为 6：去最大最小后剩 4，>>2 平均）
 --------------------------------------------------*/
#define QT_BURST_SAMPLES        6u
#define QT_BURST_AVG_SHIFT      2u

/*-------------------------------------------------
 * 滤波
 --------------------------------------------------*/
#define QT_IIR_SHIFT            2u
#define QT_NOISE_SHIFT          3u

/*-------------------------------------------------
 * 判决 / 去抖
 * T = QT_TOUCH_THRESH_MIN + noise*3
 --------------------------------------------------*/
#define QT_TOUCH_THRESH_MIN     20u
#define QT_RELEASE_HYST         8u
#define QT_DEBOUNCE_IN          3u
#define QT_DEBOUNCE_OUT         3u

/*-------------------------------------------------
 * 动态基线（未按下才跟踪；按下冻结）
 --------------------------------------------------*/
#define QT_BASELINE_UP_STEP     1u
#define QT_BASELINE_DOWN_SHIFT  5u

/*-------------------------------------------------
 * 自动校准 / 重校准
 --------------------------------------------------*/
#define QT_CAL_SAMPLES          8u
#define QT_CAL_SHIFT            3u
#define QT_RECAL_JUMP           200u
#define QT_STUCK_LIMIT          200u

#endif
