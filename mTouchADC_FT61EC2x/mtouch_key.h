#ifndef MTOUCH_KEY_H
#define MTOUCH_KEY_H

typedef struct
{
    unsigned int  signal;
    unsigned int  baseline;
    unsigned char noise;
    unsigned char pressed;
} MtKeyStatus;

void               MtKey_Init(void);
void               MtKey_Scan(void);
void               MtKey_Recalibrate(unsigned char ch);
void               MtKey_RecalibrateAll(void);
unsigned int       MtKey_GetThresh(unsigned char ch);
unsigned char      MtKey_GetPressedMask(void);
const MtKeyStatus* MtKey_GetStatus(unsigned char ch);

#endif
