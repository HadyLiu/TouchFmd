#include "soft_uart.h"
#include "delay.h"
#include "mtouch_cfg.h"

#define MT_TX_MASK (1u << MT_DBG_TX_BIT)

static void SoftUart_TxHigh(void)
{
    PORTA |= MT_TX_MASK;
}

static void SoftUart_TxLow(void)
{
    PORTA &= (unsigned char)(~MT_TX_MASK);
}

static void SoftUart_BitDelay(void)
{
    unsigned char i;

    for (i = 0u; i < MT_DBG_BAUD_DELAY; i++)
    {
        NOP();
    }
}

void SoftUart_Init(void)
{
    PORTA |= MT_TX_MASK;
    TRISA &= (unsigned char)(~MT_TX_MASK);
}

void SoftUart_PutChar(unsigned char ch)
{
    unsigned char i;

    SoftUart_TxLow();
    SoftUart_BitDelay();

    for (i = 0u; i < 8u; i++)
    {
        if (ch & 0x01u)
        {
            SoftUart_TxHigh();
        }
        else
        {
            SoftUart_TxLow();
        }
        SoftUart_BitDelay();
        ch = (unsigned char)(ch >> 1);
    }

    SoftUart_TxHigh();
    SoftUart_BitDelay();
}

void SoftUart_PutStr(const char* s)
{
    while (*s != '\0')
    {
        SoftUart_PutChar((unsigned char)(*s));
        s++;
    }
}

void SoftUart_PutU16(unsigned int v)
{
    unsigned char started;
    unsigned char dig;
    unsigned int  place;

    started = 0u;
    place   = 10000u;

    while (place > 0u)
    {
        dig = 0u;
        while (v >= place)
        {
            v = (unsigned int)(v - place);
            dig++;
        }

        if ((started != 0u) || (dig != 0u) || (place == 1u))
        {
            started = 1u;
            SoftUart_PutChar((unsigned char)('0' + dig));
        }

        if (place == 10000u)
        {
            place = 1000u;
        }
        else if (place == 1000u)
        {
            place = 100u;
        }
        else if (place == 100u)
        {
            place = 10u;
        }
        else if (place == 10u)
        {
            place = 1u;
        }
        else
        {
            place = 0u;
        }
    }
}
