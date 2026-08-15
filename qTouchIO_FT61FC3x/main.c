/*-------------------------------------------------
 * 工程：main（应用）
 * 功能：QTouch 按键扫描，按下时 PA1 输出高
 * 芯片：FT61FC3x
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"

#define QT_APP_OUT_MASK (1u << QT_APP_OUT_BIT)

static void App_PowerInit(void)
{
    OSCCON = 0B01100001; /* IRCF=110=8MHz/2T */
    OPTION = 0B00001000; /* PSA→WDT，与官网 IO 示例一致 */
    INTCON = 0;

    ANSEL0 = 0; /* 全部数字口；触摸脚禁止模拟 */

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    WPUA  = 0;
    WPUB  = 0;
    WPUC  = 0;
}

static void App_SetOut(unsigned char on)
{
    if (on != 0u)
    {
        PORTA |= QT_APP_OUT_MASK;
    }
    else
    {
        PORTA &= (unsigned char)(~QT_APP_OUT_MASK);
    }
}

void main(void)
{
    const QtKeyStatus* st;

    DelayMs(20); /* 等待上电稳定 */
    App_PowerInit();
    QtKey_Init();

    while (1)
    {
        QtKey_Scan();
        st = QtKey_GetStatus();
        App_SetOut(st->pressed);
        DelayMs(5);
    }
}
