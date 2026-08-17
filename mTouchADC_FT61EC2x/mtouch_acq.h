#ifndef MTOUCH_ACQ_H
#define MTOUCH_ACQ_H

/* 采集驱动：CVD 双极性合成。不访问寄存器。 */

void MtAcq_Init(void);

unsigned int MtAcq_MeasureOnce(unsigned char ch);
unsigned int MtAcq_Measure(unsigned char ch);

#endif
