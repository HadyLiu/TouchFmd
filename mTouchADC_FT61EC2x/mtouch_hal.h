#ifndef MTOUCH_HAL_H
#define MTOUCH_HAL_H

#include "SYSCFG.h"
#include "mtouch_cfg.h"

/* GPIO 由宏直接打到各路 PORT/TRIS/WPU；换口只改 cfg 通道表 */

#define MT_ANSEL_PRE_MASK (1u << MT_PRE_ADC)

#define MtHal_ChOutLow(ch)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((ch) == 0u)                                                                                                \
        {                                                                                                              \
            ANSEL = (unsigned char)((ANSEL & (unsigned char)(~MT_PIN_MASK(MT_CH0_ADC))) | MT_ANSEL_PRE_MASK);          \
            MT_PIN_OUT_LOW(MT_CH0_PORT, MT_CH0_TRIS, MT_CH0_WPU, MT_CH0_BIT);                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ANSEL = (unsigned char)((ANSEL & (unsigned char)(~MT_PIN_MASK(MT_CH1_ADC))) | MT_ANSEL_PRE_MASK);          \
            MT_PIN_OUT_LOW(MT_CH1_PORT, MT_CH1_TRIS, MT_CH1_WPU, MT_CH1_BIT);                                          \
        }                                                                                                              \
    } while (0)

#define MtHal_ChOutHigh(ch)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((ch) == 0u)                                                                                                \
        {                                                                                                              \
            ANSEL = (unsigned char)((ANSEL & (unsigned char)(~MT_PIN_MASK(MT_CH0_ADC))) | MT_ANSEL_PRE_MASK);          \
            MT_PIN_OUT_HIGH(MT_CH0_PORT, MT_CH0_TRIS, MT_CH0_BIT);                                                     \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ANSEL = (unsigned char)((ANSEL & (unsigned char)(~MT_PIN_MASK(MT_CH1_ADC))) | MT_ANSEL_PRE_MASK);          \
            MT_PIN_OUT_HIGH(MT_CH1_PORT, MT_CH1_TRIS, MT_CH1_BIT);                                                     \
        }                                                                                                              \
    } while (0)

#define MtHal_ChAnalog(ch)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((ch) == 0u)                                                                                                \
        {                                                                                                              \
            MT_PIN_INPUT(MT_CH0_TRIS, MT_CH0_BIT);                                                                     \
            ANSEL |= (unsigned char)(MT_PIN_MASK(MT_CH0_ADC) | MT_ANSEL_PRE_MASK);                                     \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            MT_PIN_INPUT(MT_CH1_TRIS, MT_CH1_BIT);                                                                     \
            ANSEL |= (unsigned char)(MT_PIN_MASK(MT_CH1_ADC) | MT_ANSEL_PRE_MASK);                                     \
        }                                                                                                              \
    } while (0)

void          MtHal_Init(void);
void          MtHal_BeginCh(unsigned char ch);
void          MtHal_EndCh(void);
unsigned int  MtHal_SampleCxLow(unsigned char ch);
unsigned int  MtHal_SampleCxHigh(unsigned char ch);
unsigned char MtHal_IntSaveOff(void);
void          MtHal_IntRestore(unsigned char bak);

#endif
