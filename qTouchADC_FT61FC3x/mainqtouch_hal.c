#include "qtouch_hal.h"

#define QT_SNS_MASK             (1u << QT_SNS_BIT)
#define QT_SMP_MASK             (1u << QT_SMP_BIT)

void QtHal_Init(void)
{
    /* 右对齐，SysClk/16，负参考 GND，正参考 VDD */
    ADCON1 = 0B11010101;
    ADCON2 = 0;
    ADCON3 = 0;
    ADCON0 = 0B00000001;

    QT_SNS_WPU &= (unsigned char)(~QT_SNS_MASK);
    QT_SMP_WPU &= (unsigned char)(~QT_SMP_MASK);
    QT_SNS_ANSEL &= (unsigned char)(~QT_SNS_MASK);

    QtHal_SnsOutLow();
    QtHal_SmpOutLow();
}

void QtHal_SnsOutLow(void)
{
    QT_SNS_ANSEL &= (unsigned char)(~QT_SNS_MASK);
    QT_SNS_PORT &= (unsigned char)(~QT_SNS_MASK);
    QT_SNS_TRIS &= (unsigned char)(~QT_SNS_MASK);
}

void QtHal_SnsOutHigh(void)
{
    QT_SNS_ANSEL &= (unsigned char)(~QT_SNS_MASK);
    QT_SNS_PORT |= QT_SNS_MASK;
    QT_SNS_TRIS &= (unsigned char)(~QT_SNS_MASK);
}

void QtHal_SnsInput(void)
{
    QT_SNS_ANSEL &= (unsigned char)(~QT_SNS_MASK);
    QT_SNS_TRIS |= QT_SNS_MASK;
}

void QtHal_SnsAnalog(void)
{
    QT_SNS_TRIS |= QT_SNS_MASK;
    QT_SNS_ANSEL |= QT_SNS_MASK;
}

void QtHal_SmpOutLow(void)
{
    QT_SMP_PORT &= (unsigned char)(~QT_SMP_MASK);
    QT_SMP_TRIS &= (unsigned char)(~QT_SMP_MASK);
}

void QtHal_SmpInput(void)
{
    QT_SMP_TRIS |= QT_SMP_MASK;
}

void QtHal_AdcSelectChRaw(unsigned char ch)
{
    unsigned char adcon0_buff;

    adcon0_buff = (unsigned char)(ADCON0 & 0B00000111);
    adcon0_buff |= (unsigned char)(ch << 3);
    ADCON0 = adcon0_buff;
}

unsigned int QtHal_AdcConvertRaw(void)
{
    unsigned int timeout;
    unsigned int result;

    ADCON0 = (unsigned char)(ADCON0 | 0x02u);

    timeout = QT_ADC_GO_TIMEOUT;
    while ((ADCON0 & 0x02u) != 0u)
    {
        if (timeout == 0u)
        {
            return 0u;
        }
        timeout--;
    }

    result = (unsigned int)(((unsigned int)ADRESH << 8) | (unsigned int)ADRESL);
    if (result > QT_ADC_FULLSCALE)
    {
        result = QT_ADC_FULLSCALE;
    }
    return result;
}
