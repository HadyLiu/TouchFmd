#include "qtouch_acq.h"
#include "qtouch_cfg.h"
#include "qtouch_hal.h"

void QtAcq_Init(void)
{
    QtHal_Init();
}

static void QtAcq_SettleRaw(unsigned char n)
{
    while (n > 0u)
    {
        n--;
    }
}

/*
 * 先写锁存再改方向，避免整口 RMW。
 * 充电/转移用 Settle，时间不够时采集值会乱跳、基线跟着飘。
 */
#define QT_ACQ_ONCE(sns_io, sns_dir, smp_io, smp_dir)                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        sns_io  = 0;                                                                                                   \
        smp_io  = 0;                                                                                                   \
        sns_dir = 0;                                                                                                   \
        smp_dir = 0;                                                                                                   \
        QtAcq_SettleRaw(QT_ACQ_DISCHARGE_NOPS);                                                                        \
        count = 0u;                                                                                                    \
        while (count < QT_MAX_COUNT)                                                                                   \
        {                                                                                                              \
            smp_dir = 1;                                                                                               \
            sns_io  = 1;                                                                                               \
            sns_dir = 0;                                                                                               \
            QtAcq_SettleRaw(QT_CHARGE_NOPS);                                                                           \
            sns_dir = 1;                                                                                               \
            NOP();                                                                                                     \
            smp_io  = 0;                                                                                               \
            smp_dir = 0;                                                                                               \
            QtAcq_SettleRaw(QT_TRANSFER_NOPS);                                                                         \
            if (sns_io)                                                                                                \
            {                                                                                                          \
                break;                                                                                                 \
            }                                                                                                          \
            count++;                                                                                                   \
        }                                                                                                              \
        sns_io  = 0;                                                                                                   \
        smp_io  = 0;                                                                                                   \
        sns_dir = 0;                                                                                                   \
        smp_dir = 0;                                                                                                   \
    } while (0)

static unsigned int QtAcq_OnceRaw(unsigned char ch)
{
    unsigned int count;

    if (ch == 0u)
    {
        QT_ACQ_ONCE(QT_CH0_SNS_IO, QT_CH0_SNS_DIR, QT_CH0_SMP_IO, QT_CH0_SMP_DIR);
    }
    else
    {
        QT_ACQ_ONCE(QT_CH1_SNS_IO, QT_CH1_SNS_DIR, QT_CH1_SMP_IO, QT_CH1_SMP_DIR);
    }

    return count;
}

unsigned int QtAcq_MeasureOnce(unsigned char ch)
{
    unsigned int count;

    QtHal_BeginCh(ch);
    count = QtAcq_OnceRaw(ch);
    QtHal_EndCh();

    return count;
}

unsigned int QtAcq_Measure(unsigned char ch)
{
    unsigned char int_bak;
    unsigned int  sample;

    int_bak = QtHal_IntSaveOff();
    QtHal_BeginCh(ch);
    sample = QtAcq_OnceRaw(ch);
    QtHal_EndCh();
    QtHal_IntRestore(int_bak);

    return sample;
}
