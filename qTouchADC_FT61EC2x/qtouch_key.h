#ifndef QTOUCH_KEY_H
#define QTOUCH_KEY_H

typedef struct
{
    unsigned int  signal;
    unsigned int  baseline;
    unsigned int  delta;
    unsigned char noise;
    unsigned char pressed;
} QtKeyStatus;

void               QtKey_Init(void);
void               QtKey_Scan(void);
void               QtKey_ScanCh(unsigned char ch);
void               QtKey_Recalibrate(unsigned char ch);
void               QtKey_RecalibrateAll(void);
unsigned int       QtKey_GetThresh(unsigned char ch);
unsigned char      QtKey_GetPressedMask(void);
const QtKeyStatus* QtKey_GetStatus(unsigned char ch);

#endif
