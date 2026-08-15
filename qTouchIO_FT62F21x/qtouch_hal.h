#ifndef QTOUCH_HAL_H
#define QTOUCH_HAL_H

#include "qtouch_cfg.h"

void QtHal_Init(void);

void          QtHal_SnsOutLow(void);
void          QtHal_SnsOutHigh(void);
void          QtHal_SnsInput(void);
unsigned char QtHal_SnsRead(void);

void QtHal_SmpOutLow(void);
void QtHal_SmpInput(void);

#endif
