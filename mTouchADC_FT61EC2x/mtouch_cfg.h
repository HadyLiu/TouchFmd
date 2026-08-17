#ifndef MTOUCH_CFG_H
#define MTOUCH_CFG_H

/*=================================================
 * 配置：通道表 + 算法参数。
 * 每路直接写 PORT/TRIS/WPU 寄存器名，便于换口移植。
 * 手册：AN5=PC1，AN6=PC2，AN7=内部 1/4VDD。
 *=================================================*/

#define MT_PIN_MASK(bit) ((unsigned char)(1u << (bit)))

#define MT_PIN_OUT_LOW(port, tris, wpu, bit)                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        (wpu) &= (unsigned char)(~MT_PIN_MASK(bit));                                                                   \
        (port) &= (unsigned char)(~MT_PIN_MASK(bit));                                                                  \
        (tris) &= (unsigned char)(~MT_PIN_MASK(bit));                                                                  \
    } while (0)

#define MT_PIN_OUT_HIGH(port, tris, bit)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        (port) |= MT_PIN_MASK(bit);                                                                                    \
        (tris) &= (unsigned char)(~MT_PIN_MASK(bit));                                                                  \
    } while (0)

#define MT_PIN_INPUT(tris, bit)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        (tris) |= MT_PIN_MASK(bit);                                                                                    \
    } while (0)

/*-------------------------------------------------
 * 通道表（只改这里）
 * CHn_ADC = CHS。当前：CH0 PC2(AN6)，CH1 PC1(AN5)
 --------------------------------------------------*/
#define MT_CH_MAX 2u
#define MT_CH_COUNT 2u

#define MT_CH0_PORT PORTC
#define MT_CH0_TRIS TRISC
#define MT_CH0_WPU WPUC
#define MT_CH0_BIT 2u
#define MT_CH0_ADC 6u

#define MT_CH1_PORT PORTC
#define MT_CH1_TRIS TRISC
#define MT_CH1_WPU WPUC
#define MT_CH1_BIT 1u
#define MT_CH1_ADC 5u

#define MT_PRE_ADC 7u /* 内部 1/4VDD */

#if MT_CH_COUNT < 1
#error MT_CH_COUNT must be >= 1
#endif
#if MT_CH_COUNT > MT_CH_MAX
#error MT_CH_COUNT must be <= MT_CH_MAX
#endif

#define MT_CH0_LED_BIT 7u /* PA7：通道1 按下指示，低有效 */
#define MT_CH1_LED_BIT 6u /* PA6：通道2 按下指示，低有效 */
#define MT_DBG_TX_BIT 2u /* PA2：TouchDeg 日志 TX */
#define MT_DBG_BAUD_10US 10u /* 位宽≈10*10us，对 9600；偏快减小 */

#define MT_LP_ENABLE 1u
#define MT_LP_IDLE_ONLY 1u
#define MT_LP_WDTCON 0B00001111
#define MT_LP_ACTIVE_DELAY_MS 5u

#define MT_GUARD_ENABLE 1u

#define MT_ADC_FULLSCALE 1023u
#define MT_ADC_TACQ_NOPS 8u
#define MT_ADC_GO_TIMEOUT 2000u

#define MT_BURST_SAMPLES 6u
#define MT_BURST_AVG_SHIFT 2u

#define MT_IIR_SHIFT 2u
#define MT_NOISE_SHIFT 3u

#define MT_TOUCH_THRESH_MIN 8u
#define MT_RELEASE_HYST 3u
#define MT_DEBOUNCE_IN 3u
#define MT_DEBOUNCE_OUT 3u

#define MT_BASELINE_UP_STEP 1u
#define MT_BASELINE_DOWN_SHIFT 5u

#define MT_CAL_SAMPLES 8u
#define MT_CAL_SHIFT 3u
#define MT_RECAL_JUMP 50u
#define MT_STUCK_LIMIT 200u

#endif
