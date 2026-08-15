#ifndef MTOUCH_HAL_H
#define MTOUCH_HAL_H

#include "mtouch_cfg.h"

void MtHal_Init(void);

unsigned char MtHal_GetAdcCh(unsigned char idx);

void MtHal_PinOutLow(unsigned char pin);
void MtHal_PinOutHigh(unsigned char pin);
void MtHal_PinAnalog(unsigned char pin);

void MtHal_GuardOthersRaw(unsigned char active_pin);
void MtHal_IdleAllRaw(void);

void         MtHal_AdcSelectChRaw(unsigned char ch);
unsigned int MtHal_AdcConvertRaw(void);

#endif
