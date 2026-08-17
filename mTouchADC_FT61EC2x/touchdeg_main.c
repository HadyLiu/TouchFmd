/*-------------------------------------------------
 * 工程：TouchDeg
 * 功能：逐通道上报 S/B/D/P 与按下掩码
 * 串口：PA2 软 UART 约 9600 8N1，帧 0x55 ... 0xAA
 * 指示：PA7/PA6 低有效 = 通道1/2 按下
 * 芯片：FT61EC2x 两路 CVD
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "mtouch_cfg.h"
#include "mtouch_key.h"
#include "soft_uart.h"

#define MT_CH0_LED_MASK (1u << MT_CH0_LED_BIT)
#define MT_CH1_LED_MASK (1u << MT_CH1_LED_BIT)
#define MT_LED_IDLE_MASK (MT_CH0_LED_MASK | MT_CH1_LED_MASK)

static void Deg_PowerInit(void)
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
    OPTION  = 0B00001000;
    MSCKCON = 0B00000000;
    CMCON0  = 0B00000111;
}

static void Deg_LedWriteRaw(unsigned char mask, unsigned char pressed)
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

static void Deg_UpdateLeds(unsigned char pressed_mask)
{
    Deg_LedWriteRaw(MT_CH0_LED_MASK, (unsigned char)(pressed_mask & 0x01u));
#if MT_CH_COUNT >= 2
    Deg_LedWriteRaw(MT_CH1_LED_MASK, (unsigned char)(pressed_mask & 0x02u));
#endif
}

static void Deg_PutU16(unsigned int v)
{
    SoftUart_PutChar((unsigned char)v);
    SoftUart_PutChar((unsigned char)(v >> 8));
}

/*
 * 每通道单独一帧：55 | ch | S_L S_H B_L B_H D_L D_H N P | AA
 */
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

    SoftUart_PutChar(0x55u);
    SoftUart_PutChar(ch);
    Deg_PutU16(st->signal);
    Deg_PutU16(st->baseline);
    Deg_PutU16(delta);
    SoftUart_PutChar(st->noise);
    SoftUart_PutChar(st->pressed);
    SoftUart_PutChar(0xAAu);
}

static void Deg_Report(void)
{
    unsigned char ch;

    for (ch = 0u; ch < MT_CH_COUNT; ch++)
    {
        Deg_ReportOne(ch);
    }
}

void main(void)
{
    unsigned char mask;

    DelayMs(20);
    Deg_PowerInit();
    SoftUart_Init();
    MtKey_Init();

    while (1)
    {
        MtKey_Scan();
        mask = MtKey_GetPressedMask();
        Deg_UpdateLeds(mask);
        Deg_Report();
        DelayMs(5);
    }
}
