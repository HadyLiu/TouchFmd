#include "qtouch_hal.h"

unsigned char QtHal_CurCh;

static void QtHal_ChIdleRaw(unsigned char ch)
{
    if (ch == 0u)
    {
        ANSEL &= (unsigned char)(~QT_PIN_MASK(QT_CH0_SNS_AN));
        QT_PIN_OUT_LOW(QT_CH0_SNS_PORT, QT_CH0_SNS_TRIS, QT_CH0_SNS_WPU, QT_CH0_SNS_BIT);
        QT_PIN_OUT_LOW(QT_CH0_SMP_PORT, QT_CH0_SMP_TRIS, QT_CH0_SMP_WPU, QT_CH0_SMP_BIT);
    }
    else
    {
        ANSEL &= (unsigned char)(~QT_PIN_MASK(QT_CH1_SNS_AN));
        QT_PIN_OUT_LOW(QT_CH1_SNS_PORT, QT_CH1_SNS_TRIS, QT_CH1_SNS_WPU, QT_CH1_SNS_BIT);
        QT_PIN_OUT_LOW(QT_CH1_SMP_PORT, QT_CH1_SMP_TRIS, QT_CH1_SMP_WPU, QT_CH1_SMP_BIT);
    }
}

static void QtHal_AdcSelectRaw(unsigned char adc_ch)
{
    unsigned char adcon0_buff;

    adcon0_buff = (unsigned char)(ADCON0 & 0B11100011);
    adcon0_buff |= (unsigned char)(adc_ch << 2);
    ADCON0 = adcon0_buff;
}

static unsigned int QtHal_AdcConvertRaw(void)
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

void QtHal_Nops(unsigned char n)
{
    while (n > 0u)
    {
        NOP();
        n--;
    }
}

unsigned char QtHal_IntSaveOff(void)
{
    unsigned char bak;

    bak    = INTCON;
    INTCON = 0;
    return bak;
}

void QtHal_IntRestore(unsigned char bak)
{
    INTCON = bak;
}

void QtHal_BeginCh(unsigned char ch)
{
    unsigned char i;

    if (ch >= QT_CH_COUNT)
    {
        ch = 0u;
    }
    QtHal_CurCh = ch;

    for (i = 0u; i < QT_CH_COUNT; i++)
    {
        if (i != ch)
        {
            QtHal_ChIdleRaw(i);
        }
    }
}

void QtHal_EndCh(void)
{
    QtHal_ChIdleRaw(QtHal_CurCh);
}

void QtHal_Init(void)
{
    unsigned char i;

    /* FT61EC2x：右对齐，FOSC/64，参考 VDD */
    ADCON1 = 0B01100000;
    ADCON0 = 0B10000001;

    QtHal_CurCh = 0u;
    for (i = 0u; i < QT_CH_COUNT; i++)
    {
        QtHal_ChIdleRaw(i);
    }
}

unsigned int QtHal_ReadCsAdc(void)
{
    if (QtHal_CurCh == 0u)
    {
        QT_PIN_INPUT(QT_CH0_SNS_TRIS, QT_CH0_SNS_BIT);
        ANSEL |= QT_PIN_MASK(QT_CH0_SNS_AN);
        QtHal_AdcSelectRaw(QT_CH0_SNS_AN);
    }
    else
    {
        QT_PIN_INPUT(QT_CH1_SNS_TRIS, QT_CH1_SNS_BIT);
        ANSEL |= QT_PIN_MASK(QT_CH1_SNS_AN);
        QtHal_AdcSelectRaw(QT_CH1_SNS_AN);
    }
    QtHal_Nops(QT_ADC_TACQ_NOPS);
    return QtHal_AdcConvertRaw();
}
