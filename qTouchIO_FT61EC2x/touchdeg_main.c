/*-------------------------------------------------
 * 工程：TouchDeg
 * 功能：逐通道上报 S/B/D/P 与按下掩码
 * 串口：PA2 软 UART 约 9600 8N1，帧 0x55 ... 0xAA
 * 指示：PA7/PA6 低有效 = 通道1/2 按下
 * 芯片：FT61EC2x 两路 IO QTouch
 --------------------------------------------------*/
#include "SYSCFG.h"
#include "delay.h"
#include "qtouch_cfg.h"
#include "qtouch_key.h"
#include "soft_uart.h"

#define QT_CH0_LED_MASK (1u << QT_CH0_LED_BIT)
#define QT_CH1_LED_MASK (1u << QT_CH1_LED_BIT)
#define QT_LED_IDLE_MASK (QT_CH0_LED_MASK | QT_CH1_LED_MASK)

static void Deg_PowerInit(void)
{
    OSCCON = 0B01100001;
    INTCON = 0;

    PORTA = QT_LED_IDLE_MASK;
    TRISA = 0;
    PORTC = 0;
    TRISC = 0;
    WPUA  = 0;
    WPUC  = 0;
    ANSEL = 0;

    CLRWDT();
    OPTION  = 0B00001000;
    WDTCON  = 0;
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
    Deg_LedWriteRaw(QT_CH0_LED_MASK, (unsigned char)(pressed_mask & 0x01u));
#if QT_CH_COUNT >= 2
    Deg_LedWriteRaw(QT_CH1_LED_MASK, (unsigned char)(pressed_mask & 0x02u));
#endif
}

static void Deg_PutU16(unsigned int v)
{
    SoftUart_PutChar((unsigned char)v);
    SoftUart_PutChar((unsigned char)(v >> 8));
}

static unsigned char Deg_SnrRaw(int delta, unsigned char noise)
{
    unsigned int abs_d;
    unsigned int snr;

    if (noise == 0u)
    {
        return 0u;
    }
    if (delta < 0)
    {
        abs_d = (unsigned int)(-delta);
    }
    else
    {
        abs_d = (unsigned int)delta;
    }
    snr = (unsigned int)(abs_d / (unsigned int)noise);
    if (snr > 255u)
    {
        snr = 255u;
    }
    return (unsigned char)snr;
}

/*
 * 每通道单独一帧：55 | ch | S_L S_H B_L B_H D_L D_H SNR P | AA
 */
static void Deg_ReportOne(unsigned char ch)
{
    const QtKeyStatus* st;

    st = QtKey_GetStatus(ch);

    SoftUart_PutChar(0x55u);
    SoftUart_PutChar(ch);
    Deg_PutU16(st->signal);
    Deg_PutU16(st->baseline);
    Deg_PutU16((unsigned int)st->delta);
    SoftUart_PutChar(Deg_SnrRaw(st->delta, st->noise));
    SoftUart_PutChar(st->pressed);
    SoftUart_PutChar(0xAAu);
}

void main(void)
{
    unsigned char ch;
    unsigned char mask;
    unsigned char scans;

    CLRWDT();
    DelayMs(20);
    Deg_PowerInit();
    SoftUart_Init();
    QtKey_Init();

    scans = 0u;
    while (1)
    {
        for (ch = 0u; ch < QT_CH_COUNT; ch++)
        {
            QtKey_ScanCh(ch);
            mask = QtKey_GetPressedMask();
            Deg_UpdateLeds(mask);
        }
        /* 连续扫多轮再发串口，灯跟手；9600 每帧约 12ms */
        scans++;
        if (scans >= 8u)
        {
            scans = 0u;
            for (ch = 0u; ch < QT_CH_COUNT; ch++)
            {
                Deg_ReportOne(ch);
            }
        }
    }
}
