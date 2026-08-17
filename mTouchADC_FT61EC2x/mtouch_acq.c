#include "mtouch_acq.h"
#include "mtouch_cfg.h"
#include "mtouch_hal.h"

void MtAcq_Init(void)
{
    MtHal_Init();
}

/*
 * CVD 驱动（与预充脚无关）：
 * A) Cx=0    B) Cx=VDD
 * raw = A + (满量程 - B)，触摸后下降
 */
unsigned int MtAcq_MeasureOnce(unsigned char ch)
{
    unsigned int v_a;
    unsigned int v_b;

    MtHal_BeginCh(ch);
    v_a = MtHal_SampleCxLow(ch);
    v_b = MtHal_SampleCxHigh(ch);
    MtHal_EndCh();

    return (unsigned int)(v_a + (MT_ADC_FULLSCALE - v_b));
}

unsigned int MtAcq_Measure(unsigned char ch)
{
    unsigned char i;
    unsigned char int_bak;
    unsigned int  sample;
    unsigned int  sum;
    unsigned int  vmin;
    unsigned int  vmax;

    int_bak = MtHal_IntSaveOff();

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

    MtHal_IntRestore(int_bak);

    sum = (unsigned int)(sum - vmin);
    sum = (unsigned int)(sum - vmax);

    return (unsigned int)(sum >> MT_BURST_AVG_SHIFT);
}
