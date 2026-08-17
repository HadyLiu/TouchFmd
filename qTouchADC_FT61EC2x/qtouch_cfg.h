#ifndef QTOUCH_CFG_H
#define QTOUCH_CFG_H

/*=================================================
 * 配置：通道表 + 算法参数。
 * 每路直接写 PORT/TRIS/WPU 寄存器名，便于换口移植。
 * SNS 必须是 ANx。手册：AN5=PC1，AN6=PC2，PC4 无 ADC。
 *=================================================*/

#define QT_PIN_MASK(bit) ((unsigned char)(1u << (bit)))

#define QT_PIN_OUT_LOW(port, tris, wpu, bit)                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        (wpu) &= (unsigned char)(~QT_PIN_MASK(bit));                                                                   \
        (port) &= (unsigned char)(~QT_PIN_MASK(bit));                                                                  \
        (tris) &= (unsigned char)(~QT_PIN_MASK(bit));                                                                  \
    } while (0)

#define QT_PIN_OUT_HIGH(port, tris, bit)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        (port) |= QT_PIN_MASK(bit);                                                                                    \
        (tris) &= (unsigned char)(~QT_PIN_MASK(bit));                                                                  \
    } while (0)

#define QT_PIN_INPUT(tris, bit)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        (tris) |= QT_PIN_MASK(bit);                                                                                    \
    } while (0)

/*-------------------------------------------------
 * 通道表（只改这里）
 * 当前：CH0 SNS=PC2(AN6) SMP=PC3；CH1 SNS=PC1(AN5) SMP=PC4
 --------------------------------------------------*/
#define QT_CH_MAX 2u
#define QT_CH_COUNT 2u

#define QT_CH0_SNS_PORT PORTC
#define QT_CH0_SNS_TRIS TRISC
#define QT_CH0_SNS_WPU WPUC
#define QT_CH0_SNS_BIT 2u
#define QT_CH0_SNS_AN 6u
#define QT_CH0_SMP_PORT PORTC
#define QT_CH0_SMP_TRIS TRISC
#define QT_CH0_SMP_WPU WPUC
#define QT_CH0_SMP_BIT 3u

#define QT_CH1_SNS_PORT PORTC
#define QT_CH1_SNS_TRIS TRISC
#define QT_CH1_SNS_WPU WPUC
#define QT_CH1_SNS_BIT 1u
#define QT_CH1_SNS_AN 5u
#define QT_CH1_SMP_PORT PORTC
#define QT_CH1_SMP_TRIS TRISC
#define QT_CH1_SMP_WPU WPUC
#define QT_CH1_SMP_BIT 4u

#if QT_CH_COUNT < 1
#error QT_CH_COUNT must be >= 1
#endif
#if QT_CH_COUNT > QT_CH_MAX
#error QT_CH_COUNT must be <= QT_CH_MAX
#endif

#define QT_CH0_LED_BIT 7u /* PA7：通道1 按下指示，低有效 */
#define QT_CH1_LED_BIT 6u /* PA6：通道2 按下指示，低有效 */
#define QT_DBG_TX_BIT 2u /* PA2：TouchDeg 日志 TX */
#define QT_DBG_BAUD_10US 10u /* 位宽≈10*10us，对 9600；偏快减小 */

/*-------------------------------------------------
 * 采集驱动参数
 --------------------------------------------------*/
#define QT_TRANSFER_PULSES 64u
#define QT_CHARGE_NOPS 3u
#define QT_TRANSFER_NOPS 3u
#define QT_ACQ_DISCHARGE_NOPS 6u
#define QT_ADC_TACQ_NOPS 8u
#define QT_ADC_FULLSCALE 1023u
#define QT_ADC_GO_TIMEOUT 2000u

#define QT_BURST_SAMPLES 6u
#define QT_BURST_AVG_SHIFT 2u

#define QT_IIR_SHIFT 2u
#define QT_NOISE_SHIFT 3u

#define QT_TOUCH_THRESH_MIN 8u
#define QT_RELEASE_HYST 3u
#define QT_DEBOUNCE_IN 3u
#define QT_DEBOUNCE_OUT 3u

#define QT_BASELINE_UP_STEP 1u
#define QT_BASELINE_DOWN_SHIFT 5u

#define QT_CAL_SAMPLES 8u
#define QT_CAL_SHIFT 3u
#define QT_RECAL_JUMP 50u
#define QT_STUCK_LIMIT 200u

#endif
