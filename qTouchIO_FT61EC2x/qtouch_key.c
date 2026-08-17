#include "qtouch_key.h"
#include "qtouch_acq.h"
#include "qtouch_cfg.h"

typedef struct
{
    QtKeyStatus   st;
    unsigned char debounce;
    unsigned int  press_cnt;
} QtKeyChRaw;

static QtKeyChRaw s_key[QT_CH_COUNT];

static unsigned int QtKey_ChU16(unsigned char ch, unsigned int v0,
                                unsigned int v1)
{
    if (ch == 0u)
    {
        return v0;
    }
    return v1;
}

static unsigned int QtKey_ThreshMinRaw(unsigned char ch)
{
    return QtKey_ChU16(ch, QT_CH0_THRESH_MIN, QT_CH1_THRESH_MIN);
}

unsigned int QtKey_GetThresh(unsigned char ch)
{
    unsigned int n;

    n = s_key[ch].st.noise;
    return (unsigned int)(QtKey_ThreshMinRaw(ch) + (n << 1) + n);
}

static void QtKey_UpdateNoiseRaw(unsigned char ch, unsigned int residual)
{
    unsigned int n;

    n = s_key[ch].st.noise;
    n = (unsigned int)(((n << QT_NOISE_SHIFT) - n + residual) >> QT_NOISE_SHIFT);
    if (n > 255u)
    {
        n = 255u;
    }
    s_key[ch].st.noise = (unsigned char)n;
}

static void QtKey_TrackBaselineRaw(unsigned char ch, unsigned int signal,
                                   unsigned int thresh)
{
    unsigned int delta;

    if (signal >= s_key[ch].st.baseline)
    {
        s_key[ch].st.baseline =
            (unsigned int)(s_key[ch].st.baseline + QT_BASELINE_UP_STEP);
        if (s_key[ch].st.baseline > signal)
        {
            s_key[ch].st.baseline = signal;
        }
    }
    else
    {
        delta = (unsigned int)(s_key[ch].st.baseline - signal);
        if (delta < thresh)
        {
            delta = (unsigned int)(delta >> QT_BASELINE_DOWN_SHIFT);
            if (delta == 0u)
            {
                delta = 1u;
            }
            s_key[ch].st.baseline =
                (unsigned int)(s_key[ch].st.baseline - delta);
            if (s_key[ch].st.baseline < signal)
            {
                s_key[ch].st.baseline = signal;
            }
        }
    }
}

void QtKey_Recalibrate(unsigned char ch)
{
    unsigned char i;
    unsigned int  sum;
    unsigned int  sample;
    QtKeyChRaw*   k;

    if (ch >= QT_CH_COUNT)
    {
        return;
    }

    k   = &s_key[ch];
    sum = 0u;
    for (i = 0u; i < QT_CAL_SAMPLES; i++)
    {
        sample = QtAcq_Measure(ch);
        sum    = (unsigned int)(sum + sample);
    }

    sample          = (unsigned int)(sum >> QT_CAL_SHIFT);
    k->st.baseline  = sample;
    k->st.signal    = sample;
    k->st.delta     = 0u;
    k->st.noise     = 0u;
    k->st.pressed   = 0u;
    k->debounce     = 0u;
    k->press_cnt    = 0u;
}

void QtKey_RecalibrateAll(void)
{
    unsigned char ch;

    for (ch = 0u; ch < QT_CH_COUNT; ch++)
    {
        QtKey_Recalibrate(ch);
    }
}

void QtKey_Init(void)
{
    QtAcq_Init();
    QtKey_RecalibrateAll();
}

static void QtKey_ScanOneRaw(unsigned char ch)
{
    QtKeyChRaw*  k;
    unsigned int raw;
    unsigned int filt;
    unsigned int delta;
    unsigned int thresh;
    unsigned int release_thr;
    unsigned int hyst;
    unsigned int recal_jump;
    unsigned int timeout;
    unsigned int idle_lim;
    unsigned int n;

    k   = &s_key[ch];
    raw = QtAcq_Measure(ch);
#if QT_IIR_SHIFT == 0u
    k->st.signal = raw;
#else
    filt         = k->st.signal;
    filt         = (unsigned int)(((filt << QT_IIR_SHIFT) - filt + raw) >>
                          QT_IIR_SHIFT);
    k->st.signal = filt;
    raw          = filt;
#endif
    filt = k->st.signal;

    if (k->st.baseline > filt)
    {
        delta = (unsigned int)(k->st.baseline - filt);
    }
    else
    {
        delta = 0u;
    }
    k->st.delta = delta;

    thresh     = QtKey_GetThresh(ch);
    hyst       = QtKey_ChU16(ch, QT_CH0_RELEASE_HYST, QT_CH1_RELEASE_HYST);
    recal_jump = QtKey_ChU16(ch, QT_CH0_RECAL_JUMP, QT_CH1_RECAL_JUMP);
    timeout    = QtKey_ChU16(ch, QT_CH0_PRESS_TIMEOUT, QT_CH1_PRESS_TIMEOUT);

    if (k->st.pressed == 0u)
    {
        /* 只用空闲小抖动更新 N/B；手指靠近后冻结，避免把触摸吃进去 */
        n        = k->st.noise;
        idle_lim = (unsigned int)((n << 1) + n);
        if (idle_lim < QT_NOISE_IDLE_MIN)
        {
            idle_lim = QT_NOISE_IDLE_MIN;
        }

        if ((k->debounce == 0u) && (delta <= idle_lim))
        {
            QtKey_UpdateNoiseRaw(ch, delta);
            thresh = QtKey_GetThresh(ch);
            QtKey_TrackBaselineRaw(ch, filt, thresh);
        }

        if (filt > (unsigned int)(k->st.baseline + recal_jump))
        {
            QtKey_Recalibrate(ch);
            return;
        }

        if (delta >= thresh)
        {
            if (k->debounce < 255u)
            {
                k->debounce++;
            }
            if (k->debounce >= QtKey_ChU16(ch, QT_CH0_DEBOUNCE_IN,
                                           QT_CH1_DEBOUNCE_IN))
            {
                k->st.pressed = 1u;
                k->debounce   = 0u;
                k->press_cnt  = 0u;
            }
        }
        else
        {
            k->debounce = 0u;
        }
    }
    else
    {
        if (k->press_cnt < 0xFFFFu)
        {
            k->press_cnt++;
        }
        if (k->press_cnt >= timeout)
        {
            QtKey_Recalibrate(ch);
            return;
        }

        release_thr = thresh;
        if (release_thr > hyst)
        {
            release_thr = (unsigned int)(release_thr - hyst);
        }
        else
        {
            release_thr = QtKey_ThreshMinRaw(ch);
        }

        if (delta <= release_thr)
        {
            if (k->debounce < 255u)
            {
                k->debounce++;
            }
            if (k->debounce >= QtKey_ChU16(ch, QT_CH0_DEBOUNCE_OUT,
                                           QT_CH1_DEBOUNCE_OUT))
            {
                k->st.pressed = 0u;
                k->debounce   = 0u;
                k->press_cnt  = 0u;
            }
        }
        else
        {
            k->debounce = 0u;
        }
    }
}

void QtKey_ScanCh(unsigned char ch)
{
    if (ch < QT_CH_COUNT)
    {
        QtKey_ScanOneRaw(ch);
    }
}

void QtKey_Scan(void)
{
    unsigned char ch;

    for (ch = 0u; ch < QT_CH_COUNT; ch++)
    {
        QtKey_ScanCh(ch);
    }
}

unsigned char QtKey_GetPressedMask(void)
{
    unsigned char ch;
    unsigned char mask;

    mask = 0u;
    for (ch = 0u; ch < QT_CH_COUNT; ch++)
    {
        if (s_key[ch].st.pressed != 0u)
        {
            mask |= (unsigned char)(1u << ch);
        }
    }
    return mask;
}

const QtKeyStatus* QtKey_GetStatus(unsigned char ch)
{
    if (ch >= QT_CH_COUNT)
    {
        ch = 0u;
    }
    return &s_key[ch].st;
}
