#include "qtouch_hal.h"

static unsigned char s_ch;

#define QT_PIN_IDLE(io, dir) \
    do \
    { \
        io  = 0; \
        dir = 0; \
    } while (0)

static void QtHal_ChIdleRaw(unsigned char ch)
{
    if (ch == 0u)
    {
        QT_PIN_IDLE(QT_CH0_SNS_IO, QT_CH0_SNS_DIR);
        QT_PIN_IDLE(QT_CH0_SMP_IO, QT_CH0_SMP_DIR);
    }
    else
    {
        QT_PIN_IDLE(QT_CH1_SNS_IO, QT_CH1_SNS_DIR);
        QT_PIN_IDLE(QT_CH1_SMP_IO, QT_CH1_SMP_DIR);
    }
}

unsigned char QtHal_IntSaveOff(void)
{
    unsigned char bak;

    bak    = INTCON;
    INTCON = 0;
    return bak;
}

void QtHal_IntRestore(unsigned char bak)
{
    INTCON = bak;
}

void QtHal_BeginCh(unsigned char ch)
{
    unsigned char i;

    if (ch >= QT_CH_COUNT)
    {
        ch = 0u;
    }
    s_ch = ch;

    for (i = 0u; i < QT_CH_COUNT; i++)
    {
        if (i != ch)
        {
            QtHal_ChIdleRaw(i);
        }
    }
}

void QtHal_EndCh(void)
{
    QtHal_ChIdleRaw(s_ch);
}

void QtHal_Init(void)
{
    unsigned char i;

    s_ch = 0u;
    for (i = 0u; i < QT_CH_COUNT; i++)
    {
        QtHal_ChIdleRaw(i);
    }
}
