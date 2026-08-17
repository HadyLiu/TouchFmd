/*-------------------------------------------------
 * 工程：main（应用）
 * 功能：两路 ADC CVD；PA7/PA6 低有效指示通道1/2
 * 芯片：FT61EC2x
 * 低功耗：mtouch_cfg.h 中 MT_LP_ENABLE
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "mtouch_cfg.h"
#include "mtouch_key.h"

#define MT_CH0_LED_MASK (1u << MT_CH0_LED_BIT)
#define MT_CH1_LED_MASK (1u << MT_CH1_LED_BIT)
#define MT_LED_IDLE_MASK (MT_CH0_LED_MASK | MT_CH1_LED_MASK)

static void App_PowerInit(void)
{
    OSCCON = 0B01100001;
    INTCON = 0;

    PORTA = MT_LED_IDLE_MASK;
    TRISA = 0;
    PORTC = 0;
    TRISC = 0;
    WPUA  = 0;
    WPUC  = 0;
    ANSEL = 0;

    CLRWDT();
    OPTION  = 0B00001000; /* PSA→WDT */
    MSCKCON = 0B00000000;
    CMCON0  = 0B00000111;

#if MT_LP_ENABLE
    WDTCON = MT_LP_WDTCON;
#else
    WDTCON = 0;
#endif
}

static void App_LedWriteRaw(unsigned char mask, unsigned char pressed)
{
    if (pressed != 0u)
    {
        PORTA &= (unsigned char)(~mask);
    }
    else
    {
        PORTA |= mask;
    }
}

static void App_UpdateLeds(unsigned char pressed_mask)
{
    App_LedWriteRaw(MT_CH0_LED_MASK, (unsigned char)(pressed_mask & 0x01u));
#if MT_CH_COUNT >= 2
    App_LedWriteRaw(MT_CH1_LED_MASK, (unsigned char)(pressed_mask & 0x02u));
#endif
}

#if MT_LP_ENABLE
static void App_LpSleep(void)
{
    CLRWDT();
    ADON = 0;
    SLEEP();
    NOP();
    ADON = 1;
}
#endif

void main(void)
{
    unsigned char mask;

    DelayMs(20);
    App_PowerInit();
    MtKey_Init();

    while (1)
    {
#if MT_LP_ENABLE
        CLRWDT();
#endif
        MtKey_Scan();
        mask = MtKey_GetPressedMask();
        App_UpdateLeds(mask);

#if MT_LP_ENABLE
#if MT_LP_IDLE_ONLY
        if (mask != 0u)
        {
            CLRWDT();
            DelayMs(MT_LP_ACTIVE_DELAY_MS);
        }
        else
        {
            App_LpSleep();
        }
#else
        App_LpSleep();
#endif
#else
        DelayMs(MT_LP_ACTIVE_DELAY_MS);
#endif
    }
}
