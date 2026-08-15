/*-------------------------------------------------
 * 工程：main（应用）
 * 功能：多路 ADC CVD 扫描，任意键按下时 PA1 输出高
 * 芯片：FT61FC3x
 * 低功耗：mtouch_cfg.h 中 MT_LP_ENABLE
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "mtouch_cfg.h"
#include "mtouch_key.h"

#define MT_APP_OUT_MASK (1u << MT_APP_OUT_BIT)

static void App_PowerInit(void)
{
    OSCCON = 0B01100001;
    OPTION = 0B00001000; /* PSA→WDT */
    INTCON = 0;

    ANSEL0 = 0;

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    WPUA  = 0;
    WPUB  = 0;
    WPUC  = 0;

#if MT_LP_ENABLE
    CLRWDT();
    WDTCON = MT_LP_WDTCON;
#else
    WDTCON = 0;
#endif
}

static void App_SetOut(unsigned char on)
{
    if (on != 0u)
    {
        PORTA |= MT_APP_OUT_MASK;
    }
    else
    {
        PORTA &= (unsigned char)(~MT_APP_OUT_MASK);
    }
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
        App_SetOut(mask);

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
