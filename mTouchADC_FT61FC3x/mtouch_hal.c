#include "mtouch_hal.h"

static const unsigned char s_ch_adc[MT_CH_COUNT] = MT_CH_ADC_INIT;

unsigned char MtHal_GetAdcCh(unsigned char idx)
{
    if (idx >= MT_CH_COUNT)
    {
        return s_ch_adc[0];
    }
    return s_ch_adc[idx];
}

void MtHal_Init(void)
{
    unsigned char i;

    /* 右对齐，SysClk/16，负参考 GND，正参考 VDD */
    ADCON1 = 0B11010101;
    ADCON2 = 0;
    ADCON3 = 0;
    ADCON0 = 0B00000001;

    for (i = 0u; i < MT_CH_COUNT; i++)
    {
        WPUA &= (unsigned char)(~(1u << s_ch_adc[i]));
    }
    WPUA &= (unsigned char)(~(1u << MT_PRE_ADC));

    MtHal_IdleAllRaw();
}

void MtHal_PinOutLow(unsigned char pin)
{
    unsigned char mask;

    mask = (unsigned char)(1u << pin);
    ANSEL0 &= (unsigned char)(~mask);
    PORTA &= (unsigned char)(~mask);
    TRISA &= (unsigned char)(~mask);
}

void MtHal_PinOutHigh(unsigned char pin)
{
    unsigned char mask;

    mask = (unsigned char)(1u << pin);
    ANSEL0 &= (unsigned char)(~mask);
    PORTA |= mask;
    TRISA &= (unsigned char)(~mask);
}

void MtHal_PinAnalog(unsigned char pin)
{
    unsigned char mask;

    mask = (unsigned char)(1u << pin);
    TRISA |= mask;
    ANSEL0 |= mask;
}

void MtHal_GuardOthersRaw(unsigned char active_pin)
{
    unsigned char i;
    unsigned char pin;

    for (i = 0u; i < MT_CH_COUNT; i++)
    {
        pin = s_ch_adc[i];
        if (pin != active_pin)
        {
#if MT_GUARD_ENABLE
            MtHal_PinOutLow(pin);
#else
            MtHal_PinAnalog(pin);
#endif
        }
    }
}

void MtHal_IdleAllRaw(void)
{
    unsigned char i;

    for (i = 0u; i < MT_CH_COUNT; i++)
    {
        MtHal_PinOutLow(s_ch_adc[i]);
    }
    MtHal_PinOutLow(MT_PRE_ADC);
}

void MtHal_AdcSelectChRaw(unsigned char ch)
{
    unsigned char adcon0_buff;

    adcon0_buff = (unsigned char)(ADCON0 & 0B00000111);
    adcon0_buff |= (unsigned char)(ch << 3);
    ADCON0 = adcon0_buff;
}

unsigned int MtHal_AdcConvertRaw(void)
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
