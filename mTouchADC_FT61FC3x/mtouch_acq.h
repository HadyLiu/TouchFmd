#ifndef MTOUCH_ACQ_H
#define MTOUCH_ACQ_H

void MtAcq_Init(void);

unsigned int MtAcq_MeasureOnce(unsigned char ch);
unsigned int MtAcq_Measure(unsigned char ch);

#endif
