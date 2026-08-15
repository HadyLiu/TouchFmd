#include "qtouch_key.h"
#include "qtouch_cfg.h"
#include "qtouch_acq.h"

static QtKeyStatus s_status;
static unsigned int s_med1;     /* 中值窗：med1, med2, raw */
static unsigned int s_med2;
static unsigned char s_debounce;
static unsigned char s_stuck;   /* 1 字节，配合 QT_STUCK_LIMIT<=255 */

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

unsigned int QtKey_GetThresh(void)
{
    unsigned int n;

    /* min + noise*3 = min + (noise<<1) + noise，避免乘 */
    n = s_status.noise;
    return (unsigned int)(QT_TOUCH_THRESH_MIN + (n << 1) + n);
}

static void QtKey_UpdateNoise(unsigned int residual)
{
    unsigned int n;

    /* noise = (noise*7 + residual)/8，用相对基线残差估计噪声底 */
    n = s_status.noise;
    n = (unsigned int)(((n << QT_NOISE_SHIFT) - n + residual) >> QT_NOISE_SHIFT);
    if (n > 255u)
    {
        n = 255u;
    }
    s_status.noise = (unsigned char)n;
}

static void QtKey_TrackBaseline(unsigned int signal, unsigned int thresh)
{
    unsigned int delta;

    if (signal >= s_status.baseline)
    {
        s_status.baseline =
            (unsigned int)(s_status.baseline + QT_BASELINE_UP_STEP);
        if (s_status.baseline > signal)
        {
            s_status.baseline = signal;
        }
    }
    else
    {
        delta = (unsigned int)(s_status.baseline - signal);
        if (delta < thresh)
        {
            delta = (unsigned int)(delta >> QT_BASELINE_DOWN_SHIFT);
            if (delta == 0u)
            {
                delta = 1u;
            }
            s_status.baseline = (unsigned int)(s_status.baseline - delta);
            if (s_status.baseline < signal)
            {
                s_status.baseline = signal;
            }
        }
    }
}

void QtKey_Recalibrate(void)
{
    unsigned char i;
    unsigned int sum;
    unsigned int sample;

    sum = 0u;
    for (i = 0u; i < QT_CAL_SAMPLES; i++)
    {
        sample = QtAcq_Measure();
        sum = (unsigned int)(sum + sample);
    }

    sample = (unsigned int)(sum >> QT_CAL_SHIFT);
    s_status.baseline = sample;
    s_status.signal = sample;
    s_status.noise = 0u;
    s_status.pressed = 0u;

    s_med1 = sample;
    s_med2 = sample;
    s_debounce = 0u;
    s_stuck = 0u;
}

void QtKey_Init(void)
{
    QtAcq_Init();
    QtKey_Recalibrate();
}

void QtKey_Scan(void)
{
    unsigned int raw;
    unsigned int med;
    unsigned int filt;
    unsigned int delta;
    unsigned int thresh;
    unsigned int release_thr;

    raw = QtAcq_Measure();

    /* 三点滑动中值 */
    med = QtKey_Median3(s_med1, s_med2, raw);
    s_med1 = s_med2;
    s_med2 = raw;

    /* IIR：filt = (filt*3 + med)/4 */
    filt = s_status.signal;
    filt = (unsigned int)(((filt << QT_IIR_SHIFT) - filt + med) >> QT_IIR_SHIFT);
    s_status.signal = filt;

    if (s_status.baseline > filt)
    {
        delta = (unsigned int)(s_status.baseline - filt);
    }
    else
    {
        delta = 0u;
    }

    thresh = QtKey_GetThresh();

    if (s_status.pressed == 0u)
    {
        /* 未按下：用未过阈值的残差更新噪声 */
        if (delta < thresh)
        {
            QtKey_UpdateNoise(delta);
            thresh = QtKey_GetThresh();
        }
    }

    /* 异常正向跳变 -> 重校准 */
    if (filt > (unsigned int)(s_status.baseline + QT_RECAL_JUMP))
    {
        QtKey_Recalibrate();
        return;
    }

    if (s_status.pressed == 0u)
    {
        QtKey_TrackBaseline(filt, thresh);

        if (delta >= thresh)
        {
            if (s_debounce < 255u)
            {
                s_debounce++;
            }
            if (s_debounce >= QT_DEBOUNCE_IN)
            {
                s_status.pressed = 1u;
                s_debounce = 0u;
                s_stuck = 0u;
            }
        }
        else
        {
            s_debounce = 0u;
        }
    }
    else
    {
        if (s_stuck < 255u)
        {
            s_stuck++;
        }
        if (s_stuck >= QT_STUCK_LIMIT)
        {
            QtKey_Recalibrate();
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
            if (s_debounce < 255u)
            {
                s_debounce++;
            }
            if (s_debounce >= QT_DEBOUNCE_OUT)
            {
                s_status.pressed = 0u;
                s_debounce = 0u;
                s_stuck = 0u;
            }
        }
        else
        {
            s_debounce = 0u;
        }
    }
}

const QtKeyStatus *QtKey_GetStatus(void)
{
    return &s_status;
}
