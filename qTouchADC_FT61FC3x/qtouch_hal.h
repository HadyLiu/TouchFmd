#ifndef QTOUCH_HAL_H
#define QTOUCH_HAL_H

#include "qtouch_cfg.h"

void QtHal_Init(void);

void QtHal_SnsOutLow(void);
void QtHal_SnsOutHigh(void);
void QtHal_SnsInput(void);
void QtHal_SnsAnalog(void);

void QtHal_SmpOutLow(void);
void QtHal_SmpInput(void);

void QtHal_AdcSelectChRaw(unsigned char ch);
unsigned int QtHal_AdcConvertRaw(void);

#endif
