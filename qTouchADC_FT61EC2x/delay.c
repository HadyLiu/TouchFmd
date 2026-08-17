#include "delay.h"

/* FT61EC2x：8MHz/2T，按官网 IO 示例校准 */
void Delay10Us(void)
{
    unsigned char i;

    for (i = 0; i < 3; i++)
    {
        NOP();
        NOP();
        NOP();
        NOP();
    }
}

void DelayMs(unsigned char Time)
{
    unsigned int  a;
    unsigned char b;

    for (a = 0; a < Time; a++)
    {
        for (b = 0; b < 97; b++)
        {
            Delay10Us();
        }
    }
}
