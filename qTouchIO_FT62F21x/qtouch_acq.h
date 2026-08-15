#ifndef QTOUCH_ACQ_H
#define QTOUCH_ACQ_H

void QtAcq_Init(void);

/* 单次电荷转移测量 */
unsigned int QtAcq_MeasureOnce(void);

/*
 * 突发采样：关中断 + 去最大最小再平均，提升 SNR
 */
unsigned int QtAcq_Measure(void);

#endif
