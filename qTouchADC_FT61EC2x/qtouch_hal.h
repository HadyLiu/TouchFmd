#ifndef QTOUCH_HAL_H
#define QTOUCH_HAL_H

#include "SYSCFG.h"
#include "qtouch_cfg.h"

/* GPIO 由宏直接打到各路 PORT/TRIS/WPU；换口只改 cfg 通道表 */

extern unsigned char QtHal_CurCh;

#define QtHal_SnsAnselOff()                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            ANSEL &= (unsigned char)(~QT_PIN_MASK(QT_CH0_SNS_AN));                                                     \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ANSEL &= (unsigned char)(~QT_PIN_MASK(QT_CH1_SNS_AN));                                                     \
        }                                                                                                              \
    } while (0)

#define QtHal_SnsOutLow()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        QtHal_SnsAnselOff();                                                                                           \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            QT_PIN_OUT_LOW(QT_CH0_SNS_PORT, QT_CH0_SNS_TRIS, QT_CH0_SNS_WPU, QT_CH0_SNS_BIT);                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            QT_PIN_OUT_LOW(QT_CH1_SNS_PORT, QT_CH1_SNS_TRIS, QT_CH1_SNS_WPU, QT_CH1_SNS_BIT);                          \
        }                                                                                                              \
    } while (0)

#define QtHal_SnsOutHigh()                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        QtHal_SnsAnselOff();                                                                                           \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            QT_PIN_OUT_HIGH(QT_CH0_SNS_PORT, QT_CH0_SNS_TRIS, QT_CH0_SNS_BIT);                                         \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            QT_PIN_OUT_HIGH(QT_CH1_SNS_PORT, QT_CH1_SNS_TRIS, QT_CH1_SNS_BIT);                                         \
        }                                                                                                              \
    } while (0)

#define QtHal_SnsInput()                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        QtHal_SnsAnselOff();                                                                                           \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            QT_PIN_INPUT(QT_CH0_SNS_TRIS, QT_CH0_SNS_BIT);                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            QT_PIN_INPUT(QT_CH1_SNS_TRIS, QT_CH1_SNS_BIT);                                                             \
        }                                                                                                              \
    } while (0)

#define QtHal_SmpOutLow()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            QT_PIN_OUT_LOW(QT_CH0_SMP_PORT, QT_CH0_SMP_TRIS, QT_CH0_SMP_WPU, QT_CH0_SMP_BIT);                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            QT_PIN_OUT_LOW(QT_CH1_SMP_PORT, QT_CH1_SMP_TRIS, QT_CH1_SMP_WPU, QT_CH1_SMP_BIT);                          \
        }                                                                                                              \
    } while (0)

#define QtHal_SmpInput()                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (QtHal_CurCh == 0u)                                                                                         \
        {                                                                                                              \
            QT_PIN_INPUT(QT_CH0_SMP_TRIS, QT_CH0_SMP_BIT);                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            QT_PIN_INPUT(QT_CH1_SMP_TRIS, QT_CH1_SMP_BIT);                                                             \
        }                                                                                                              \
    } while (0)

void          QtHal_Init(void);
void          QtHal_BeginCh(unsigned char ch);
void          QtHal_EndCh(void);
unsigned int  QtHal_ReadCsAdc(void);
unsigned char QtHal_IntSaveOff(void);
void          QtHal_IntRestore(unsigned char bak);
void          QtHal_Nops(unsigned char n);

#endif
