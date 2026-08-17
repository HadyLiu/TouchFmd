#include "soft_uart.h"
#include "delay.h"
#include "qtouch_cfg.h"

#define QT_TX_MASK (1u << QT_DBG_TX_BIT)

static void SoftUart_TxHigh(void)
{
    PORTA |= QT_TX_MASK;
}

static void SoftUart_TxLow(void)
{
    PORTA &= (unsigned char)(~QT_TX_MASK);
}

static void SoftUart_BitDelay(void)
{
    unsigned char i;

    /* FT61EC2x 8MHz/2T：10*Delay10Us ≈ 100us，对齐 9600 的 104us */
    for (i = 0u; i < QT_DBG_BAUD_10US; i++)
    {
        Delay10Us();
    }
}

void SoftUart_Init(void)
{
    PORTA |= QT_TX_MASK;
    TRISA &= (unsigned char)(~QT_TX_MASK);
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
