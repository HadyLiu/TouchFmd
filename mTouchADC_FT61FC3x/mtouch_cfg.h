#ifndef MTOUCH_CFG_H
#define MTOUCH_CFG_H

#include "SYSCFG.h"

/*=================================================
 * mtouch_cfg.h — FT61FC3x 多路 ADC CVD
 *
 * 改通道：只改 MT_CH_COUNT 与 MT_CH_ADC_INIT。
 * 元素 = ANx = PAx 位号（AN0=PA0 … AN7=PA7）。
 * 不要与 PRE、PA1(TX/指示)、PA6(ISP CLK) 重复。
 *
 * 例：4 路 PA0/PA3/PA4/PA5
 *   #define MT_CH_COUNT     4u
 *   #define MT_CH_ADC_INIT  { 0u, 3u, 4u, 5u }
 *=================================================*/

#define MT_CH_MAX 8u
#define MT_CH_COUNT 3u
#define MT_CH_ADC_INIT {0u, 3u, 4u}

#if MT_CH_COUNT < 1
#error MT_CH_COUNT must be >= 1
#endif
#if MT_CH_COUNT > 8
#error MT_CH_COUNT must be <= 8
#endif

/* PRE：C_hold 预充 dummy，不接铜箔，不要出现在 MT_CH_ADC_INIT 里 */
#define MT_PRE_ADC 2u /* PA2 / AN2 */

#define MT_APP_OUT_BIT 1u /* 任意键按下 → PA1 高 */
#define MT_DBG_TX_BIT 1u  /* TouchDeg 软串口 TX = PA1 */
#define MT_DBG_BAUD_DELAY 33u

/*-------------------------------------------------
 * 低功耗（仅 main.prj）
 * MT_LP_ENABLE     1=WDT+SLEEP；0=DelayMs 忙等
 * MT_LP_IDLE_ONLY  1=仅空闲休眠（按下用 DelayMs 跟手）
 * MT_LP_WDTCON     SWDTEN=1, WDTPS=0111, LIRC 32k
 *                  (4096/32000)≈128ms，须大于一次全通道扫描
 --------------------------------------------------*/
#define MT_LP_ENABLE 1u
#define MT_LP_IDLE_ONLY 1u
#define MT_LP_WDTCON 0B00001111
#define MT_LP_ACTIVE_DELAY_MS 5u

/*-------------------------------------------------
 * Guard：测量某通道时处理其它感应脚
 * 1=拉低作地屏蔽，减串扰；0=其它脚模拟高阻，不屏蔽
 --------------------------------------------------*/
#define MT_GUARD_ENABLE 1u

/*-------------------------------------------------
 * ADC / CVD
 * raw = Va + (4095 - Vb)，触摸后下降
 --------------------------------------------------*/
#define MT_ADC_FULLSCALE 4095u
#define MT_ADC_TACQ_NOPS 8u
#define MT_ADC_GO_TIMEOUT 2000u

#define MT_BURST_SAMPLES 6u
#define MT_BURST_AVG_SHIFT 2u

#define MT_IIR_SHIFT 2u
#define MT_NOISE_SHIFT 3u

#define MT_TOUCH_THRESH_MIN 20u
#define MT_RELEASE_HYST 8u
#define MT_DEBOUNCE_IN 3u
#define MT_DEBOUNCE_OUT 3u

#define MT_BASELINE_UP_STEP 1u
#define MT_BASELINE_DOWN_SHIFT 5u

#define MT_CAL_SAMPLES 8u
#define MT_CAL_SHIFT 3u
#define MT_RECAL_JUMP 200u
#define MT_STUCK_LIMIT 200u

#endif
