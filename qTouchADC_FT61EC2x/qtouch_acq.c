#include "qtouch_acq.h"
#include "qtouch_cfg.h"
#include "qtouch_hal.h"

void QtAcq_Init(void)
{
    QtHal_Init();
}

/*
 * 电荷转移驱动：固定次数后由 HAL 读 Cs。
 * raw = FULLSCALE - adc，触摸后下降。
 */
unsigned int QtAcq_MeasureOnce(unsigned char ch)
{
    unsigned char i;
    unsigned int  adc;

    QtHal_BeginCh(ch);

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();
    QtHal_Nops(QT_ACQ_DISCHARGE_NOPS);

    for (i = 0u; i < QT_TRANSFER_PULSES; i++)
    {
        QtHal_SmpInput();
        QtHal_SnsOutHigh();
        QtHal_Nops(QT_CHARGE_NOPS);

        QtHal_SnsInput();
        QtHal_SmpOutLow();
        QtHal_Nops(QT_TRANSFER_NOPS);
    }

    adc = QtHal_ReadCsAdc();
    QtHal_EndCh();

    return (unsigned int)(QT_ADC_FULLSCALE - adc);
}

unsigned int QtAcq_Measure(unsigned char ch)
{
    unsigned char i;
    unsigned char int_bak;
    unsigned int  sample;
    unsigned int  sum;
    unsigned int  vmin;
    unsigned int  vmax;

    int_bak = QtHal_IntSaveOff();

    sum  = 0u;
    vmin = 0xFFFFu;
    vmax = 0u;

    for (i = 0u; i < QT_BURST_SAMPLES; i++)
    {
        sample = QtAcq_MeasureOnce(ch);
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

    QtHal_IntRestore(int_bak);

    sum = (unsigned int)(sum - vmin);
    sum = (unsigned int)(sum - vmax);

    return (unsigned int)(sum >> QT_BURST_AVG_SHIFT);
}
