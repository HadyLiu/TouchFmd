#include "qtouch_key.h"
#include "qtouch_acq.h"
#include "qtouch_cfg.h"

static QtKeyStatus   s_st[QT_CH_COUNT];
static unsigned int  s_med1[QT_CH_COUNT];
static unsigned int  s_med2[QT_CH_COUNT];
static unsigned char s_debounce[QT_CH_COUNT];
static unsigned char s_stuck[QT_CH_COUNT];

static unsigned int QtKey_Median3(unsigned int a, unsigned int b, unsigned int c)
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

unsigned int QtKey_GetThresh(unsigned char ch)
{
    unsigned int n;

    n = s_st[ch].noise;
    return (unsigned int)(QT_TOUCH_THRESH_MIN + (n << 1) + n);
}

static void QtKey_UpdateNoiseRaw(unsigned char ch, unsigned int residual)
{
    unsigned int n;

    n = s_st[ch].noise;
    n = (unsigned int)(((n << QT_NOISE_SHIFT) - n + residual) >> QT_NOISE_SHIFT);
    if (n < 1u)
    {
        n = 1u;
    }
    if (n > 255u)
    {
        n = 255u;
    }
    s_st[ch].noise = (unsigned char)n;
}

static void QtKey_TrackBaselineRaw(unsigned char ch, unsigned int signal,
                                   unsigned int thresh)
{
    unsigned int delta;

    if (signal >= s_st[ch].baseline)
    {
        s_st[ch].baseline = (unsigned int)(s_st[ch].baseline + QT_BASELINE_UP_STEP);
        if (s_st[ch].baseline > signal)
        {
            s_st[ch].baseline = signal;
        }
    }
    else
    {
        delta = (unsigned int)(s_st[ch].baseline - signal);
        if (delta < thresh)
        {
            delta = (unsigned int)(delta >> QT_BASELINE_DOWN_SHIFT);
            if (delta == 0u)
            {
                delta = 1u;
            }
            s_st[ch].baseline = (unsigned int)(s_st[ch].baseline - delta);
            if (s_st[ch].baseline < signal)
            {
                s_st[ch].baseline = signal;
            }
        }
    }
}

void QtKey_Recalibrate(unsigned char ch)
{
    unsigned char i;
    unsigned int  sum;
    unsigned int  sample;

    if (ch >= QT_CH_COUNT)
    {
        return;
    }

    sum = 0u;
    for (i = 0u; i < QT_CAL_SAMPLES; i++)
    {
        sample = QtAcq_Measure(ch);
        sum    = (unsigned int)(sum + sample);
    }

    sample            = (unsigned int)(sum >> QT_CAL_SHIFT);
    s_st[ch].baseline = sample;
    s_st[ch].signal   = sample;
    s_st[ch].noise    = 1u;
    s_st[ch].pressed  = 0u;
    s_med1[ch]        = sample;
    s_med2[ch]        = sample;
    s_debounce[ch]    = 0u;
    s_stuck[ch]       = 0u;
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
    unsigned int raw;
    unsigned int med;
    unsigned int filt;
    unsigned int delta;
    unsigned int thresh;
    unsigned int release_thr;

    raw = QtAcq_Measure(ch);

    med        = QtKey_Median3(s_med1[ch], s_med2[ch], raw);
    s_med1[ch] = s_med2[ch];
    s_med2[ch] = raw;

    filt            = s_st[ch].signal;
    filt            = (unsigned int)(((filt << QT_IIR_SHIFT) - filt + med) >> QT_IIR_SHIFT);
    s_st[ch].signal = filt;

    if (s_st[ch].baseline > filt)
    {
        delta = (unsigned int)(s_st[ch].baseline - filt);
    }
    else
    {
        delta = 0u;
    }

    thresh = QtKey_GetThresh(ch);

    if (s_st[ch].pressed == 0u)
    {
        if (delta < thresh)
        {
            QtKey_UpdateNoiseRaw(ch, delta);
            thresh = QtKey_GetThresh(ch);
        }
    }

    if (filt > (unsigned int)(s_st[ch].baseline + QT_RECAL_JUMP))
    {
        QtKey_Recalibrate(ch);
        return;
    }

    if (s_st[ch].pressed == 0u)
    {
        QtKey_TrackBaselineRaw(ch, filt, thresh);

        if (delta >= thresh)
        {
            if (s_debounce[ch] < 255u)
            {
                s_debounce[ch]++;
            }
            if (s_debounce[ch] >= QT_DEBOUNCE_IN)
            {
                s_st[ch].pressed = 1u;
                s_debounce[ch]   = 0u;
                s_stuck[ch]      = 0u;
            }
        }
        else
        {
            s_debounce[ch] = 0u;
        }
    }
    else
    {
        if (s_stuck[ch] < 255u)
        {
            s_stuck[ch]++;
        }
        if (s_stuck[ch] >= QT_STUCK_LIMIT)
        {
            QtKey_Recalibrate(ch);
            return;
        }

        release_thr = thresh;
        if (release_thr > QT_RELEASE_HYST)
        {
            release_thr = (unsigned int)(release_thr - QT_RELEASE_HYST);
        }
        else
        {
            release_thr = QT_TOUCH_THRESH_MIN;
        }

        if (delta <= release_thr)
        {
            if (s_debounce[ch] < 255u)
            {
                s_debounce[ch]++;
            }
            if (s_debounce[ch] >= QT_DEBOUNCE_OUT)
            {
                s_st[ch].pressed = 0u;
                s_debounce[ch]   = 0u;
                s_stuck[ch]      = 0u;
            }
        }
        else
        {
            s_debounce[ch] = 0u;
        }
    }
}

void QtKey_Scan(void)
{
    unsigned char ch;

    for (ch = 0u; ch < QT_CH_COUNT; ch++)
    {
        QtKey_ScanOneRaw(ch);
    }
}

unsigned char QtKey_GetPressedMask(void)
{
    unsigned char ch;
    unsigned char mask;

    mask = 0u;
    for (ch = 0u; ch < QT_CH_COUNT; ch++)
    {
        if (s_st[ch].pressed != 0u)
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
    return &s_st[ch];
}
