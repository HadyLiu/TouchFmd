#ifndef QTOUCH_ACQ_H
#define QTOUCH_ACQ_H

void QtAcq_Init(void);

/* 固定次数电荷转移 + 一次 ADC */
unsigned int QtAcq_MeasureOnce(void);

/*
 * 突发采样：关中断 + 去最大最小再平均，提升 SNR
 */
unsigned int QtAcq_Measure(void);

#endif
