#include "mtouch_acq.h"
#include "mtouch_cfg.h"
#include "mtouch_hal.h"

static void MtAcq_NopsRaw(unsigned char n)
{
    while (n > 0u)
    {
        NOP();
        n--;
    }
}

void MtAcq_Init(void)
{
    MtHal_Init();
}

/*
 * 双极性 CVD：
 * A) C_hold=VDD, Cx=0 → 触摸后 ADC 下降
 * B) C_hold=0,   Cx=VDD → 触摸后 ADC 上升
 * raw = A + (满量程 - B)，触摸后下降
 */
unsigned int MtAcq_MeasureOnce(unsigned char ch)
{
    unsigned char sns;
    unsigned int  v_a;
    unsigned int  v_b;

    sns = MtHal_GetAdcCh(ch);
    MtHal_GuardOthersRaw(sns);

    /* A: PRE 预充 VDD，电极拉低 */
    MtHal_PinOutHigh(MT_PRE_ADC);
    MtHal_PinAnalog(MT_PRE_ADC);
    MtHal_AdcSelectChRaw(MT_PRE_ADC);
    MtAcq_NopsRaw(MT_ADC_TACQ_NOPS);

    MtHal_PinOutLow(sns);
    MtHal_PinAnalog(sns);
    MtHal_AdcSelectChRaw(sns);
    MtAcq_NopsRaw(MT_ADC_TACQ_NOPS);
    v_a = MtHal_AdcConvertRaw();

    /* B: PRE 预充 0，电极拉高 */
    MtHal_PinOutLow(MT_PRE_ADC);
    MtHal_PinAnalog(MT_PRE_ADC);
    MtHal_AdcSelectChRaw(MT_PRE_ADC);
    MtAcq_NopsRaw(MT_ADC_TACQ_NOPS);

    MtHal_PinOutHigh(sns);
    MtHal_PinAnalog(sns);
    MtHal_AdcSelectChRaw(sns);
    MtAcq_NopsRaw(MT_ADC_TACQ_NOPS);
    v_b = MtHal_AdcConvertRaw();

    MtHal_IdleAllRaw();

    return (unsigned int)(v_a + (MT_ADC_FULLSCALE - v_b));
}

unsigned int MtAcq_Measure(unsigned char ch)
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

    for (i = 0u; i < MT_BURST_SAMPLES; i++)
    {
        sample = MtAcq_MeasureOnce(ch);
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

    sum = (unsigned int)(sum - vmin);
    sum = (unsigned int)(sum - vmax);

    return (unsigned int)(sum >> MT_BURST_AVG_SHIFT);
}
