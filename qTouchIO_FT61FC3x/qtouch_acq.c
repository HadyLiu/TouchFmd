#include "qtouch_acq.h"
#include "qtouch_cfg.h"
#include "qtouch_hal.h"

static void QtAcq_NopsRaw(unsigned char n)
{
    while (n > 0u)
    {
        NOP();
        n--;
    }
}

void QtAcq_Init(void)
{
    QtHal_Init();
}

/*
 * 1) SNS/SMP 拉低泄放 Cs
 * 2) SMP 高阻，SNS 输出高，给 Cx 充电
 * 3) SNS 高阻，SMP 拉低，电荷转入 Cs
 * 4) 读 SNS 是否到约 1/2VCC(VIH)
 */
unsigned int QtAcq_MeasureOnce(void)
{
    unsigned int count;

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();
    QtAcq_NopsRaw(QT_ACQ_DISCHARGE_NOPS);

    count = 0u;
    while (count < QT_MAX_COUNT)
    {
        QtHal_SmpInput();
        QtHal_SnsOutHigh();
        QtAcq_NopsRaw(QT_CHARGE_NOPS);

        QtHal_SnsInput();
        QtHal_SmpOutLow();
        QtAcq_NopsRaw(QT_TRANSFER_NOPS);

        if (QtHal_SnsRead() != 0u)
        {
            break;
        }
        count++;
    }

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();

    return count;
}

unsigned int QtAcq_Measure(void)
{
    unsigned char i;
    unsigned char intcon_bak;
    unsigned int  sample;
    unsigned int  sum;
    unsigned int  vmin;
    unsigned int  vmax;

    intcon_bak = INTCON;
    INTCON     = 0;

    sum  = 0u;
    vmin = 0xFFFFu;
    vmax = 0u;

    for (i = 0u; i < QT_BURST_SAMPLES; i++)
    {
        sample = QtAcq_MeasureOnce();
        sum    = (unsigned int)(sum + sample);
        if (sample < vmin)
        {
            vmin = sample;
        }
        if (sample > vmax)
        {
            vmax = sample;
        }
    }

    INTCON = intcon_bak;

    /* 去最大最小后剩 4 点，>>2 代替 /4 */
    sum = (unsigned int)(sum - vmin);
    sum = (unsigned int)(sum - vmax);

    return (unsigned int)(sum >> QT_BURST_AVG_SHIFT);
}
