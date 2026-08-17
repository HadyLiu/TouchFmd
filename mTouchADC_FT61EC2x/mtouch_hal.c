#include "mtouch_hal.h"

static void MtHal_NopsRaw(unsigned char n)
{
    while (n > 0u)
    {
        NOP();
        n--;
    }
}

static void MtHal_AdcSelectRaw(unsigned char adc_ch)
{
    unsigned char adcon0_buff;

    adcon0_buff = (unsigned char)(ADCON0 & 0B11100011);
    adcon0_buff |= (unsigned char)(adc_ch << 2);
    ADCON0 = adcon0_buff;
}

static unsigned int MtHal_AdcConvertRaw(void)
{
    unsigned int timeout;
    unsigned int result;

    ADCON0 = (unsigned char)(ADCON0 | 0x02u);

    timeout = MT_ADC_GO_TIMEOUT;
    while ((ADCON0 & 0x02u) != 0u)
    {
        if (timeout == 0u)
        {
            return MT_ADC_FULLSCALE;
        }
        timeout--;
    }

    result = (unsigned int)(((unsigned int)ADRESH << 8) | (unsigned int)ADRESL);
    if (result > MT_ADC_FULLSCALE)
    {
        result = MT_ADC_FULLSCALE;
    }
    return result;
}

static unsigned int MtHal_ShareConvertRaw(unsigned char ch)
{
    MtHal_ChAnalog(ch);
    if (ch == 0u)
    {
        MtHal_AdcSelectRaw(MT_CH0_ADC);
    }
    else
    {
        MtHal_AdcSelectRaw(MT_CH1_ADC);
    }
    MtHal_NopsRaw(MT_ADC_TACQ_NOPS);
    return MtHal_AdcConvertRaw();
}

unsigned char MtHal_IntSaveOff(void)
{
    unsigned char bak;

    bak    = INTCON;
    INTCON = 0;
    return bak;
}

void MtHal_IntRestore(unsigned char bak)
{
    INTCON = bak;
}

void MtHal_BeginCh(unsigned char ch)
{
    unsigned char i;

    for (i = 0u; i < MT_CH_COUNT; i++)
    {
        if (i != ch)
        {
#if MT_GUARD_ENABLE
            MtHal_ChOutLow(i);
#else
            MtHal_ChAnalog(i);
#endif
        }
    }
}

void MtHal_EndCh(void)
{
    unsigned char i;

    for (i = 0u; i < MT_CH_COUNT; i++)
    {
        MtHal_ChOutLow(i);
    }
}

void MtHal_Init(void)
{
    /* FT61EC2x：右对齐，FOSC/64，参考 VDD */
    ADCON1 = 0B01100000;
    ADCON0 = 0B10000001;
    ANSEL  = MT_ANSEL_PRE_MASK;

    MtHal_EndCh();
}

unsigned int MtHal_SampleCxLow(unsigned char ch)
{
    MtHal_ChOutLow(ch);
    MtHal_AdcSelectRaw(MT_PRE_ADC);
    MtHal_NopsRaw(MT_ADC_TACQ_NOPS);
    return MtHal_ShareConvertRaw(ch);
}

unsigned int MtHal_SampleCxHigh(unsigned char ch)
{
    MtHal_ChOutHigh(ch);
    MtHal_AdcSelectRaw(MT_PRE_ADC);
    MtHal_NopsRaw(MT_ADC_TACQ_NOPS);
    return MtHal_ShareConvertRaw(ch);
}
