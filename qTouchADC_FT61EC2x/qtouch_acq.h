#ifndef QTOUCH_ACQ_H
#define QTOUCH_ACQ_H

/* 采集驱动：电荷转移时序。不访问寄存器。 */

void QtAcq_Init(void);

unsigned int QtAcq_MeasureOnce(unsigned char ch);
unsigned int QtAcq_Measure(unsigned char ch);

#endif
