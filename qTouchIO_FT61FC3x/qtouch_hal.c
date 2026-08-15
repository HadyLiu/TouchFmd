#include "qtouch_hal.h"

#define QT_SNS_MASK (1u << QT_SNS_BIT)
#define QT_SMP_MASK (1u << QT_SMP_BIT)

void QtHal_Init(void)
{
    /* SNS/SMP 关上拉，先输出低电平泄放 */
    QT_SNS_WPU &= (unsigned char)(~QT_SNS_MASK);
    QT_SMP_WPU &= (unsigned char)(~QT_SMP_MASK);

    QT_SNS_PORT &= (unsigned char)(~QT_SNS_MASK);
    QT_SMP_PORT &= (unsigned char)(~QT_SMP_MASK);

    QT_SNS_TRIS &= (unsigned char)(~QT_SNS_MASK);
    QT_SMP_TRIS &= (unsigned char)(~QT_SMP_MASK);
}

void QtHal_SnsOutLow(void)
{
    QT_SNS_PORT &= (unsigned char)(~QT_SNS_MASK);
    QT_SNS_TRIS &= (unsigned char)(~QT_SNS_MASK);
}

void QtHal_SnsOutHigh(void)
{
    QT_SNS_PORT |= QT_SNS_MASK;
    QT_SNS_TRIS &= (unsigned char)(~QT_SNS_MASK);
}

void QtHal_SnsInput(void)
{
    QT_SNS_TRIS |= QT_SNS_MASK;
}

unsigned char QtHal_SnsRead(void)
{
    if (QT_SNS_PORT & QT_SNS_MASK)
    {
        return 1u;
    }
    return 0u;
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
