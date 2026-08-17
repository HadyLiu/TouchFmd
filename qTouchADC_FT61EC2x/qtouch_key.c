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

unsigned int QtKey_GetThresh(unsigned char ch)
{
    return QtKey_ChU16(ch, QT_CH0_THRESH_MIN, QT_CH1_THRESH_MIN);
}

static void QtKey_UpdateNoiseRaw(unsigned char ch, unsigned int residual)
{
    unsigned int n;

    n = s_key[ch].st.noise;
    /* (7N+|D|+4)/8，+4 才能从 0 爬起来；N=空闲 |B-S| */
    n = (unsigned int)(((n << QT_NOISE_SHIFT) - n + residual + 4u) >>
                       QT_NOISE_SHIFT);
    if (n > 255u)
    {
        n = 255u;
    }
    s_key[ch].st.noise = (unsigned char)n;
}

static void QtKey_TrackBaselineRaw(unsigned char ch, unsigned int signal)
{
    unsigned int b;
    unsigned int step;

    b = s_key[ch].st.baseline;
    if (signal >= b)
    {
        b = (unsigned int)(b + QT_BASELINE_UP_STEP);
        if (b > signal)
        {
            b = signal;
        }
    }
    else
    {
        step = (unsigned int)((b - signal) >> QT_BASELINE_DOWN_SHIFT);
        if (step == 0u)
        {
            step = 1u;
        }
        b = (unsigned int)(b - step);
        if (b < signal)
        {
            b = signal;
        }
    }
    s_key[ch].st.baseline = b;
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

    sample         = (unsigned int)(sum >> QT_CAL_SHIFT);
    k->st.baseline = sample;
    k->st.signal   = sample;
    k->st.delta    = 0;
    k->st.noise    = 0u;
    k->st.pressed  = 0u;
    k->debounce    = 0u;
    k->press_cnt   = 0u;
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

static unsigned int QtKey_ShapePressRaw(unsigned int prev, unsigned int raw)
{
    unsigned int rise;

    if (raw <= prev)
    {
        return raw;
    }

    rise = (unsigned int)(raw - prev);
    if (rise > QT_PRESS_OSC_MAX)
    {
        return raw;
    }

    rise = (unsigned int)(rise >> QT_SIGNAL_UP_SHIFT);
    if (rise == 0u)
    {
        rise = 1u;
    }
    prev = (unsigned int)(prev + rise);
    if (prev > raw)
    {
        prev = raw;
    }
    return prev;
}

static void QtKey_ScanOneRaw(unsigned char ch)
{
    QtKeyChRaw*  k;
    unsigned int raw;
    unsigned int s;
    unsigned int abs_d;
    unsigned int idle_lim;
    unsigned int n;
    int          d;
    unsigned int thresh;
    unsigned int hyst;
    unsigned int release_thr;
    unsigned int recal_jump;
    unsigned int timeout;

    k   = &s_key[ch];
    raw = QtAcq_Measure(ch);
    s   = raw;
    if (k->st.pressed != 0u)
    {
        s = QtKey_ShapePressRaw(k->st.signal, raw);
    }
    k->st.signal = s;
    d            = (int)k->st.baseline - (int)s;
    k->st.delta  = d;
    if (d < 0)
    {
        abs_d = (unsigned int)(-d);
    }
    else
    {
        abs_d = (unsigned int)d;
    }

    thresh     = QtKey_GetThresh(ch);
    hyst       = QtKey_ChU16(ch, QT_CH0_RELEASE_HYST, QT_CH1_RELEASE_HYST);
    recal_jump = QtKey_ChU16(ch, QT_CH0_RECAL_JUMP, QT_CH1_RECAL_JUMP);
    timeout    = QtKey_ChU16(ch, QT_CH0_PRESS_TIMEOUT, QT_CH1_PRESS_TIMEOUT);

    if (k->st.pressed == 0u)
    {
        n        = (unsigned int)k->st.noise;
        idle_lim = (unsigned int)((n << 1) + QT_NOISE_IDLE_MIN);
        /* 只吃空闲 |D|；手指靠近（|D| 突然比 2N 大）不抬 N */
        if (d < (int)thresh)
        {
            if ((n == 0u) || (abs_d <= idle_lim))
            {
                QtKey_UpdateNoiseRaw(ch, abs_d);
                n = (unsigned int)k->st.noise;
            }
        }

        idle_lim = QT_NOISE_IDLE_MIN;
        if (n > idle_lim)
        {
            idle_lim = n;
        }

        /* 空闲才跟 B；D 超过空闲带则冻 B，避免吃掉手指 */
        if (d <= (int)idle_lim)
        {
            QtKey_TrackBaselineRaw(ch, s);
        }

        if (s > (unsigned int)(k->st.baseline + recal_jump))
        {
            QtKey_Recalibrate(ch);
            return;
        }

        if (d >= (int)thresh)
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
        else if (k->debounce > 0u)
        {
            k->debounce--;
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

        if (d <= (int)release_thr)
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
        else if (k->debounce > 0u)
        {
            k->debounce--;
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
