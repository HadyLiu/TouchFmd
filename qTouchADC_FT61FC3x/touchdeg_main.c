/*-------------------------------------------------
 * 工程：TouchDeg（调试上报）
 * 功能：上报 signal / baseline / delta / noise / thresh / pressed
 * 串口：PA1 软 UART 约 9600 8N1
 * 格式：S=.. B=.. D=.. N=.. T=.. P=..\r\n
 * 芯片：FT61FC3x ADC CVD
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"
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

static void Deg_Report(void)
{
    const QtKeyStatus* st;
    unsigned int delta;

    st = QtKey_GetStatus();
    if (st->baseline > st->signal)
    {
        delta = (unsigned int)(st->baseline - st->signal);
    }
    else
    {
        delta = 0u;
    }

    SoftUart_PutStr("S=");
    SoftUart_PutU16(st->signal);
    SoftUart_PutStr(" B=");
    SoftUart_PutU16(st->baseline);
    SoftUart_PutStr(" D=");
    SoftUart_PutU16(delta);
    SoftUart_PutStr(" N=");
    SoftUart_PutU16(st->noise);
    SoftUart_PutStr(" T=");
    SoftUart_PutU16(QtKey_GetThresh());
    SoftUart_PutStr(" P=");
    SoftUart_PutU16(st->pressed);
    SoftUart_PutStr("\r\n");
}

void main(void)
{
    DelayMs(20);
    Deg_PowerInit();
    SoftUart_Init();
    QtKey_Init();

    SoftUart_PutStr("QTouch ADC Deg\r\n");

    while (1)
    {
        QtKey_Scan();
        Deg_Report();
        DelayMs(50);
    }
}
