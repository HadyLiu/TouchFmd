/*-------------------------------------------------
 * 工程：TouchDeg
 * 功能：逐通道上报 S/B/D/P 与按下掩码
 * 串口：PA1 软 UART 约 9600 8N1
 * 芯片：FT61FC3x 多路 CVD
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "mtouch_cfg.h"
#include "mtouch_key.h"
#include "soft_uart.h"

static void Deg_PowerInit(void)
{
    OSCCON = 0B01100001;
    OPTION = 0B00001000;
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
}

static void Deg_ReportOne(unsigned char ch)
{
    const MtKeyStatus* st;
    unsigned int       delta;

    st = MtKey_GetStatus(ch);
    if (st->baseline > st->signal)
    {
        delta = (unsigned int)(st->baseline - st->signal);
    }
    else
    {
        delta = 0u;
    }

    SoftUart_PutStr("K");
    SoftUart_PutU16(ch);
    SoftUart_PutStr(" S=");
    SoftUart_PutU16(st->signal);
    SoftUart_PutStr(" B=");
    SoftUart_PutU16(st->baseline);
    SoftUart_PutStr(" D=");
    SoftUart_PutU16(delta);
    SoftUart_PutStr(" P=");
    SoftUart_PutU16(st->pressed);
    SoftUart_PutStr("\r\n");
}

void main(void)
{
    unsigned char ch;

    DelayMs(20);
    Deg_PowerInit();
    SoftUart_Init();
    MtKey_Init();

    SoftUart_PutStr("mTouch CVD Deg\r\n");

    while (1)
    {
        MtKey_Scan();
        for (ch = 0u; ch < MT_CH_COUNT; ch++)
        {
            Deg_ReportOne(ch);
        }
        SoftUart_PutStr("M=");
        SoftUart_PutU16(MtKey_GetPressedMask());
        SoftUart_PutStr("\r\n");
        DelayMs(50);
    }
}
