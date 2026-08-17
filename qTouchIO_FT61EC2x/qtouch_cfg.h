#ifndef QTOUCH_CFG_H
#define QTOUCH_CFG_H

/* 通道表 + 算法参数。换口、调参只改本文件。 */

#define QT_CH_MAX 2u
#define QT_CH_COUNT 2u /* 使用通道数，1 或 2 */

#define QT_CH0_SNS_IO PC2 /* CH0 SNS=PC2 */
#define QT_CH0_SNS_DIR TRISC2
#define QT_CH0_SMP_IO PC3 /* CH0 SMP=PC3 */
#define QT_CH0_SMP_DIR TRISC3

#define QT_CH1_SNS_IO PC1 /* CH1 SNS=PC1 */
#define QT_CH1_SNS_DIR TRISC1
#define QT_CH1_SMP_IO PC4 /* CH1 SMP=PC4 */
#define QT_CH1_SMP_DIR TRISC4

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

#define QT_MAX_COUNT 1000u       /* 单次最大转移次数；越大范围越大、越慢 */
#define QT_CHARGE_NOPS 2u        /* Cx 充电稳定 NOP */
#define QT_TRANSFER_NOPS 2u      /* Cx→Cs 转移稳定 NOP */
#define QT_BURST_SAMPLES 1u      /* 每轮采集次数；1=最快 */
#define QT_ACQ_DISCHARGE_NOPS 6u /* 测量前泄放 Cs 的 NOP */

#define QT_IIR_SHIFT 0u       /* 保留宏；按键层已固定用当次 S */
#define QT_NOISE_SHIFT 3u     /* N IIR：/8；空闲 |D| 慢平均 */
#define QT_NOISE_IDLE_MIN 4u  /* 空闲带下限；实际带子=max(本值, N) */
#define QT_PRESS_OSC_MAX 40u  /* 按下后 S 上升超过此=松开，立刻跟上 */
#define QT_SIGNAL_UP_SHIFT 3u /* 按下后小幅上浮 >>3，压门限抖动 */

#define QT_BASELINE_UP_STEP 1u    /* 空闲且未过阈值：S>B 每轮跟上 */
#define QT_BASELINE_DOWN_SHIFT 5u /* 空闲且未过阈值：下漂 >>5，慢跟环境 */
#define QT_CAL_SAMPLES 4u         /* 上电/重校准采样次数 */
#define QT_CAL_SHIFT 2u           /* 4 点 >>2 平均 */

/* CH0 独立参数 */
#define QT_CH0_THRESH 140u
#define QT_CH0_THRESH_MIN (QT_CH0_THRESH * 65 / 100)   /* 固定按下阈值 T */
#define QT_CH0_RELEASE_HYST (QT_CH0_THRESH * 40 / 100) /* 释放回差 */
#define QT_CH0_DEBOUNCE_IN 1u                          /* 未使用；按下当次过 T 即有效 */
#define QT_CH0_DEBOUNCE_OUT 2u                         /* 松开消抖次数 */
#define QT_CH0_RECAL_JUMP 40u                          /* 空闲 S 突升重校准 */
#define QT_CH0_PRESS_TIMEOUT 500u                      /* 长按超时扫描次数 */

/* CH1 独立参数 */
#define QT_CH1_THRESH 140u
#define QT_CH1_THRESH_MIN (QT_CH1_THRESH * 65 / 100)   /* 固定按下阈值 T */
#define QT_CH1_RELEASE_HYST (QT_CH1_THRESH * 40 / 100) /* 释放回差 */
#define QT_CH1_DEBOUNCE_IN 1u                          /* 未使用；按下当次过 T 即有效 */
#define QT_CH1_DEBOUNCE_OUT 2u                         /* 松开消抖次数 */
#define QT_CH1_RECAL_JUMP 40u                          /* 空闲 S 突升重校准 */
#define QT_CH1_PRESS_TIMEOUT 500u                      /* 长按超时扫描次数 */

#endif
