/*-------------------------------------------------
 * 工程：TouchDeg（调试上报）
 * 功能：上报 signal / baseline / delta / noise / pressed
 * 串口：PA1 软 UART 约 9600 8N1
 * 帧：55 | ch | S_L S_H B_L B_H D_L D_H N P | AA
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"
#include "soft_uart.h"

static void Deg_PowerInit(void)
{
    OSCCON = 0B01100001;
    INTCON = 0;
    OPTION = 0;

    TRISA  = 0;
    PORTA  = 0;
    WPUA   = 0;
    PSRCA  = 0;
    PSINKA = 0;
}

static void Deg_PutU16(unsigned int v)
{
    SoftUart_PutChar((unsigned char)v);
    SoftUart_PutChar((unsigned char)(v >> 8));
}

static void Deg_Report(void)
{
    const QtKeyStatus* st;
    unsigned int       delta;

    st = QtKey_GetStatus();
    if (st->baseline > st->signal)
    {
        delta = (unsigned int)(st->baseline - st->signal);
    }
    else
    {
        delta = 0u;
    }

    SoftUart_PutChar(0x55u);
    SoftUart_PutChar(0u);
    Deg_PutU16(st->signal);
    Deg_PutU16(st->baseline);
    Deg_PutU16(delta);
    SoftUart_PutChar(st->noise);
    SoftUart_PutChar(st->pressed);
    SoftUart_PutChar(0xAAu);
}

void main(void)
{
    Deg_PowerInit();
    SoftUart_Init();
    QtKey_Init();

    while (1)
    {
        QtKey_Scan();
        Deg_Report();
        DelayMs(50);
    }
}
