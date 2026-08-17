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

static unsigned int QtAcq_Median3Raw(unsigned int a, unsigned int b,
                                     unsigned int c)
{
    unsigned int t;

    if (a > b)
    {
        t = a;
        a = b;
        b = t;
    }
    if (b > c)
    {
        t = b;
        b = c;
        c = t;
    }
    if (a > b)
    {
        t = a;
        a = b;
        b = t;
    }
    return b;
}

unsigned int QtAcq_Measure(unsigned char ch)
{
    unsigned char int_bak;
    unsigned int  a;
#if QT_BURST_SAMPLES >= 2
    unsigned int  b;
#endif
#if QT_BURST_SAMPLES >= 3
    unsigned int  c;
#endif

    int_bak = QtHal_IntSaveOff();
    QtHal_BeginCh(ch);
    a = QtAcq_OnceRaw(ch);
#if QT_BURST_SAMPLES >= 2
    b = QtAcq_OnceRaw(ch);
#endif
#if QT_BURST_SAMPLES >= 3
    c = QtAcq_OnceRaw(ch);
#endif
    QtHal_EndCh();
    QtHal_IntRestore(int_bak);

#if QT_BURST_SAMPLES >= 3
    return QtAcq_Median3Raw(a, b, c);
#elif QT_BURST_SAMPLES >= 2
    return (unsigned int)((a + b) >> 1);
#else
    return a;
#endif
}
