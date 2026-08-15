#ifndef QTOUCH_KEY_H
#define QTOUCH_KEY_H

typedef struct
{
    unsigned int  signal;   /* IIR 后信号 */
    unsigned int  baseline; /* 动态基线 */
    unsigned char noise;    /* 噪声底估计 */
    unsigned char pressed;  /* 1=按下 */
} QtKeyStatus;

void               QtKey_Init(void);
void               QtKey_Scan(void);
void               QtKey_Recalibrate(void);
unsigned int       QtKey_GetThresh(void);
const QtKeyStatus* QtKey_GetStatus(void);

#endif
