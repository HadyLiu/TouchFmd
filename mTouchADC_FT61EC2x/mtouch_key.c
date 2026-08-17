#include "mtouch_key.h"
#include "mtouch_acq.h"
#include "mtouch_cfg.h"

static MtKeyStatus   s_st[MT_CH_COUNT];
static unsigned int  s_med1[MT_CH_COUNT];
static unsigned int  s_med2[MT_CH_COUNT];
static unsigned char s_debounce[MT_CH_COUNT];
static unsigned char s_stuck[MT_CH_COUNT];

static unsigned int MtKey_Median3(unsigned int a, unsigned int b, unsigned int c)
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

unsigned int MtKey_GetThresh(unsigned char ch)
{
    unsigned int n;

    n = s_st[ch].noise;
    return (unsigned int)(MT_TOUCH_THRESH_MIN + (n << 1) + n);
}

static void MtKey_UpdateNoiseRaw(unsigned char ch, unsigned int residual)
{
    unsigned int n;

    n = s_st[ch].noise;
    n = (unsigned int)(((n << MT_NOISE_SHIFT) - n + residual) >> MT_NOISE_SHIFT);
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

static void MtKey_TrackBaselineRaw(unsigned char ch, unsigned int signal, unsigned int thresh)
{
    unsigned int delta;

    if (signal >= s_st[ch].baseline)
    {
        s_st[ch].baseline = (unsigned int)(s_st[ch].baseline + MT_BASELINE_UP_STEP);
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
            delta = (unsigned int)(delta >> MT_BASELINE_DOWN_SHIFT);
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

void MtKey_Recalibrate(unsigned char ch)
{
    unsigned char i;
    unsigned int  sum;
    unsigned int  sample;

    if (ch >= MT_CH_COUNT)
    {
        return;
    }

    sum = 0u;
    for (i = 0u; i < MT_CAL_SAMPLES; i++)
    {
        sample = MtAcq_Measure(ch);
        sum    = (unsigned int)(sum + sample);
    }

    sample            = (unsigned int)(sum >> MT_CAL_SHIFT);
    s_st[ch].baseline = sample;
    s_st[ch].signal   = sample;
    s_st[ch].noise    = 1u;
    s_st[ch].pressed  = 0u;
    s_med1[ch]        = sample;
    s_med2[ch]        = sample;
    s_debounce[ch]    = 0u;
    s_stuck[ch]       = 0u;
}

void MtKey_RecalibrateAll(void)
{
    unsigned char ch;

    for (ch = 0u; ch < MT_CH_COUNT; ch++)
    {
        MtKey_Recalibrate(ch);
    }
}

void MtKey_Init(void)
{
    MtAcq_Init();
    MtKey_RecalibrateAll();
}

static void MtKey_ScanOneRaw(unsigned char ch)
{
    unsigned int raw;
    unsigned int med;
    unsigned int filt;
    unsigned int delta;
    unsigned int thresh;
    unsigned int release_thr;

    raw = MtAcq_Measure(ch);

    med        = MtKey_Median3(s_med1[ch], s_med2[ch], raw);
    s_med1[ch] = s_med2[ch];
    s_med2[ch] = raw;

    filt            = s_st[ch].signal;
    filt            = (unsigned int)(((filt << MT_IIR_SHIFT) - filt + med) >> MT_IIR_SHIFT);
    s_st[ch].signal = filt;

    if (s_st[ch].baseline > filt)
    {
        delta = (unsigned int)(s_st[ch].baseline - filt);
    }
    else
    {
        delta = 0u;
    }

    thresh = MtKey_GetThresh(ch);

    if (s_st[ch].pressed == 0u)
    {
        if (delta < thresh)
        {
            MtKey_UpdateNoiseRaw(ch, delta);
            thresh = MtKey_GetThresh(ch);
        }
    }

    if (filt > (unsigned int)(s_st[ch].baseline + MT_RECAL_JUMP))
    {
        MtKey_Recalibrate(ch);
        return;
    }

    if (s_st[ch].pressed == 0u)
    {
        MtKey_TrackBaselineRaw(ch, filt, thresh);

        if (delta >= thresh)
        {
            if (s_debounce[ch] < 255u)
            {
                s_debounce[ch]++;
            }
            if (s_debounce[ch] >= MT_DEBOUNCE_IN)
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
        if (s_stuck[ch] >= MT_STUCK_LIMIT)
        {
            MtKey_Recalibrate(ch);
            return;
        }

        release_thr = thresh;
        if (release_thr > MT_RELEASE_HYST)
        {
            release_thr = (unsigned int)(release_thr - MT_RELEASE_HYST);
        }
        else
        {
            release_thr = MT_TOUCH_THRESH_MIN;
        }

        if (delta <= release_thr)
        {
            if (s_debounce[ch] < 255u)
            {
                s_debounce[ch]++;
            }
            if (s_debounce[ch] >= MT_DEBOUNCE_OUT)
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

void MtKey_Scan(void)
{
    unsigned char ch;

    for (ch = 0u; ch < MT_CH_COUNT; ch++)
    {
        MtKey_ScanOneRaw(ch);
    }
}

unsigned char MtKey_GetPressedMask(void)
{
    unsigned char ch;
    unsigned char mask;

    mask = 0u;
    for (ch = 0u; ch < MT_CH_COUNT; ch++)
    {
        if (s_st[ch].pressed != 0u)
        {
            mask |= (unsigned char)(1u << ch);
        }
    }
    return mask;
}

const MtKeyStatus* MtKey_GetStatus(unsigned char ch)
{
    if (ch >= MT_CH_COUNT)
    {
        ch = 0u;
    }
    return &s_st[ch];
}
