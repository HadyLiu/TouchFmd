#ifndef QTOUCH_CFG_H
#define QTOUCH_CFG_H

/* 通道表 + 算法参数。换口、调参只改本文件。
 * SNS 必须是 ANx。手册：AN5=PC1，AN6=PC2，PC4 无 ADC。 */

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

#define QT_CH_MAX 2u
#define QT_CH_COUNT 2u /* 使用通道数，1 或 2 */

#define QT_CH0_SNS_PORT PORTC /* CH0 SNS 口 */
#define QT_CH0_SNS_TRIS TRISC
#define QT_CH0_SNS_WPU WPUC
#define QT_CH0_SNS_BIT 2u /* PC2 */
#define QT_CH0_SNS_AN 6u  /* AN6，须为 ANx */
#define QT_CH0_SMP_PORT PORTC
#define QT_CH0_SMP_TRIS TRISC
#define QT_CH0_SMP_WPU WPUC
#define QT_CH0_SMP_BIT 3u /* PC3 */

#define QT_CH1_SNS_PORT PORTC /* CH1 SNS 口 */
#define QT_CH1_SNS_TRIS TRISC
#define QT_CH1_SNS_WPU WPUC
#define QT_CH1_SNS_BIT 1u /* PC1 */
#define QT_CH1_SNS_AN 5u  /* AN5 */
#define QT_CH1_SMP_PORT PORTC
#define QT_CH1_SMP_TRIS TRISC
#define QT_CH1_SMP_WPU WPUC
#define QT_CH1_SMP_BIT 4u /* PC4，无 ADC，只作 SMP */

#if QT_CH_COUNT < 1
#error QT_CH_COUNT must be >= 1
#endif
#if QT_CH_COUNT > QT_CH_MAX
#error QT_CH_COUNT must be <= QT_CH_MAX
#endif

#define QT_CH0_LED_BIT 7u    /* PA7：CH0 按下指示，低有效 */
#define QT_CH1_LED_BIT 6u    /* PA6：CH1 按下指示，低有效 */
#define QT_DBG_TX_BIT 2u     /* PA2：TouchDeg TX */
#define QT_DBG_BAUD_10US 10u /* 位宽≈10*10us→9600；偏快减小 */

#define QT_TRANSFER_PULSES 128u  /* 固定转移次数；空闲 ADC 太小则加大 */
#define QT_CHARGE_NOPS 3u        /* Cx 充电稳定 NOP */
#define QT_TRANSFER_NOPS 3u      /* Cx→Cs 转移稳定 NOP */
#define QT_ACQ_DISCHARGE_NOPS 6u /* 测量前泄放 Cs 的 NOP */
#define QT_ADC_TACQ_NOPS 8u      /* ADC 采样保持时间 NOP */
#define QT_ADC_FULLSCALE 1023u   /* 10 位满量程；S=本值-adc */
#define QT_ADC_GO_TIMEOUT 2000u  /* 等 GO 完成的循环上限 */

#define QT_BURST_SAMPLES 1u   /* 每路每轮采集次数；1=最快跟手 */
#define QT_BURST_AVG_SHIFT 0u /* 突发>1 且去极值后用；1 次时无效 */

#define QT_IIR_SHIFT 0u      /* 0：S=当次采集，跟手最快 */
#define QT_NOISE_SHIFT 3u    /* 噪声底 IIR：/8 */
#define QT_NOISE_IDLE_MIN 4u /* 空闲噪声带下限；超过则冻 B/N */
#define QT_RECAL_HOLD 8u     /* S 连续高出 JUMP 这么多次才重校准 */

#define QT_BASELINE_UP_STEP 2u    /* 空闲 S>B：每轮跟上；ADC 跟上要快一点 */
#define QT_BASELINE_DOWN_SHIFT 5u /* 空闲且未过阈值：下漂 >>5，慢跟环境 */
#define QT_CAL_SAMPLES 4u         /* 上电/重校准采样次数 */
#define QT_CAL_SHIFT 2u           /* 4 点 >>2 平均 */

/* CH0 独立参数（ADC 差量尺度，按板调 THRESH） */
#define QT_CH0_THRESH 40u                              /* 标称差量；MIN/HYST 由此算出 */
#define QT_CH0_THRESH_MIN (QT_CH0_THRESH * 65 / 100)   /* 最小阈值；T=本值+本路N×3 */
#define QT_CH0_RELEASE_HYST (QT_CH0_THRESH * 40 / 100) /* 释放回差 */
#define QT_CH0_DEBOUNCE_IN 1u                          /* 按下消抖次数 */
#define QT_CH0_DEBOUNCE_OUT 1u                         /* 松开消抖次数 */
#define QT_CH0_RECAL_JUMP 200u                         /* 空闲 S 突升；须连续才重校准 */
#define QT_CH0_PRESS_TIMEOUT 2000u                     /* 长按超时扫描次数 */

/* CH1 独立参数 */
#define QT_CH1_THRESH 40u                              /* 标称差量；MIN/HYST 由此算出 */
#define QT_CH1_THRESH_MIN (QT_CH1_THRESH * 65 / 100)   /* 最小阈值；T=本值+本路N×3 */
#define QT_CH1_RELEASE_HYST (QT_CH1_THRESH * 40 / 100) /* 释放回差 */
#define QT_CH1_DEBOUNCE_IN 1u                          /* 按下消抖次数 */
#define QT_CH1_DEBOUNCE_OUT 1u                         /* 松开消抖次数 */
#define QT_CH1_RECAL_JUMP 200u                         /* 空闲 S 突升；须连续才重校准 */
#define QT_CH1_PRESS_TIMEOUT 2000u                     /* 长按超时扫描次数 */

#endif
