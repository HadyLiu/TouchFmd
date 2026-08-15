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
 * 与 IO 方案相同的电荷转移，但只做 QT_TRANSFER_PULSES 次，
 * 然后 ADC 读 Cs，不必等到数字口 VIH。
 * 触摸 → Cx 大 → 同样次数下 Cs 电压高 → ADC 大。
 * raw = FULLSCALE - adc，触摸后下降，供 qtouch_key 使用。
 */
unsigned int QtAcq_MeasureOnce(void)
{
    unsigned char i;
    unsigned int adc;

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();
    QtAcq_NopsRaw(QT_ACQ_DISCHARGE_NOPS);

    for (i = 0u; i < QT_TRANSFER_PULSES; i++)
    {
        QtHal_SmpInput();
        QtHal_SnsOutHigh();
        QtAcq_NopsRaw(QT_CHARGE_NOPS);

        QtHal_SnsInput();
        QtHal_SmpOutLow();
        QtAcq_NopsRaw(QT_TRANSFER_NOPS);
    }

    /* SMP 保持低，SNS 改为模拟，读 Cs 电压 */
    QtHal_SnsAnalog();
    QtHal_AdcSelectChRaw(QT_SNS_ADC_CH);
    QtAcq_NopsRaw(QT_ADC_TACQ_NOPS);
    adc = QtHal_AdcConvertRaw();

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();

    return (unsigned int)(QT_ADC_FULLSCALE - adc);
}

unsigned int QtAcq_Measure(void)
{
    unsigned char i;
    unsigned char intcon_bak;
    unsigned int sample;
    unsigned int sum;
    unsigned int vmin;
    unsigned int vmax;

    intcon_bak = INTCON;
    INTCON = 0;

    sum = 0u;
    vmin = 0xFFFFu;
    vmax = 0u;

    for (i = 0u; i < QT_BURST_SAMPLES; i++)
    {
        sample = QtAcq_MeasureOnce();
        sum = (unsigned int)(sum + sample);
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

    sum = (unsigned int)(sum - vmin);
    sum = (unsigned int)(sum - vmax);

    return (unsigned int)(sum >> QT_BURST_AVG_SHIFT);
}
