/*-------------------------------------------------
 * 工程：main（应用）
 * 功能：QTouch 按键扫描，按下时 PA1 输出高
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"

#define QT_APP_OUT_MASK (1u << QT_APP_OUT_BIT)

static void App_PowerInit(void)
{
    OSCCON = 0B01100001; /* 内部 8MHz */
    INTCON = 0;
    OPTION = 0;

    /* 默认口线输出低，触摸脚由 QtHal 接管 */
    TRISA  = 0;
    PORTA  = 0;
    WPUA   = 0;
    PSRCA  = 0;
    PSINKA = 0;
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
