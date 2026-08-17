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
    unsigned int i;
    unsigned int adc;

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
    unsigned char int_bak;
    unsigned int  sample;

    int_bak = QtHal_IntSaveOff();
    sample  = QtAcq_MeasureOnce(ch);
    QtHal_IntRestore(int_bak);

    return sample;
}
