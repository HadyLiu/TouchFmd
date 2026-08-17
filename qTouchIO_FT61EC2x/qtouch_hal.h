#ifndef QTOUCH_HAL_H
#define QTOUCH_HAL_H

#include "SYSCFG.h"
#include "qtouch_cfg.h"

void          QtHal_Init(void);
void          QtHal_BeginCh(unsigned char ch);
void          QtHal_EndCh(void);
unsigned char QtHal_IntSaveOff(void);
void          QtHal_IntRestore(unsigned char bak);

#endif
