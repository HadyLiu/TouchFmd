#include "delay.h"

void Delay10Us(void)
{
    unsigned char i;

    for (i = 0; i < 2; i++)
    {
        NOP();
    }
}

void DelayMs(unsigned char Time)
{
    unsigned int  a;
    unsigned char b;

    for (a = 0; a < Time; a++)
    {
        for (b = 0; b < 83; b++)
        {
            Delay10Us();
        }
    }
}
