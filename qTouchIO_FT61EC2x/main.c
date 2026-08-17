/*-------------------------------------------------
 * 工程：main（应用）
 * 功能：两路 QTouch；PA7/PA6 低有效指示通道1/2
 * 芯片：FT61EC2x
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"

#define QT_CH0_LED_MASK (1u << QT_CH0_LED_BIT)
#define QT_CH1_LED_MASK (1u << QT_CH1_LED_BIT)
#define QT_LED_IDLE_MASK (QT_CH0_LED_MASK | QT_CH1_LED_MASK)

static void App_PowerInit(void)
{
    OSCCON = 0B01100001; /* IRCF=110=8MHz/2T */
    INTCON = 0;

    PORTA = QT_LED_IDLE_MASK; /* 指示脚空闲为高（低有效） */
    TRISA = 0;
    PORTC = 0;
    TRISC = 0;
    WPUA  = 0;
    WPUC  = 0;
    ANSEL = 0;

    CLRWDT();
    OPTION  = 0B00001000; /* PSA→WDT */
    WDTCON  = 0;          /* 应用工程关掉 WDT，避免校准被复位 */
    MSCKCON = 0B00000000;
    CMCON0  = 0B00000111; /* 比较器关，CxIN 为数字口 */
}

/* 低有效：按下拉低，松开拉高 */
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
    App_LedWriteRaw(QT_CH0_LED_MASK, (unsigned char)(pressed_mask & 0x01u));
#if QT_CH_COUNT >= 2
    App_LedWriteRaw(QT_CH1_LED_MASK, (unsigned char)(pressed_mask & 0x02u));
#endif
}

void main(void)
{
    unsigned char ch;
    unsigned char mask;

    CLRWDT();
    DelayMs(20);
    App_PowerInit();
    QtKey_Init();

    while (1)
    {
        for (ch = 0u; ch < QT_CH_COUNT; ch++)
        {
            QtKey_ScanCh(ch);
            mask = QtKey_GetPressedMask();
            App_UpdateLeds(mask);
        }
    }
}
