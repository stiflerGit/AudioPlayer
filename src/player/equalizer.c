/**
 * @file equalizer.c
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief implementation of an audio equalizer
 * @version 0.1
 * @date 2019-03-19
 * 
 * This file contains the implementation of an audio equalizer
 * by means of four parallel peakingEQ filters
 */
#include "player/equalizer.h"

#include <error.h>
#include <math.h>

static int audio_frequency; /**< sampling frequency of the input signal. */

const int equalizer_freq[NFILT] = {250, 2000, 5000, 10000}; /**< center frequencies of filters*/

/**
 * @brief contains all coefficients needed to filter a sample.
 * 
 * Coeficient needed for filter a stream of time-data audio signal.
 * As explained in:
 * @ref https://www.w3.org/2011/audio/audio-eq-cookbook.html
 */
typedef struct filter filter_t;
struct filter
{
    float g;
    float S_BW;
    float w0;
    float s, c;
    float a1, a2;
    float b0, b1, b2;
    float xmem1, xmem2;
    float ymem1, ymem2;
    void (*calc_coef)(filter_t *);
};

/**
 * @brief set the gain of the filter
 * 
 * @param f 
 * @param gain 
 */
static void filt_set_gain(filter_t *f, float gain)
{
    f->g = gain;
    f->calc_coef(f);
}

/**
 * @brief filter a single sample if time data
 * 
 * @param[in] i 	index of the filter to use for filtering
 * @param[in] x 	sample of time data
 * @return float 	resulting filtered sample
 */
static float filter_filt(filter_t *f, float x)
{
    double y;

    y = f->b0 * x +
        f->b1 * f->xmem1 +
        f->b2 * f->xmem2 -
        f->a1 * f->ymem1 -
        f->a2 * f->ymem2;

    f->xmem2 = f->xmem1;
    f->xmem1 = x;
    f->ymem2 = f->ymem1;
    f->ymem1 = y;

    return y;
}

/**
 * @brief filter a buffer(stream) of time data
 * 
 * @param[in] i			index of the filter to use for filtering
 * @param[inout] buf	time-data samples 
 * @param count 		number of samples in buf
 */
static void filter_filtb(filter_t *f, float buf[], unsigned int count)
{
    int j;

    for (j = 0; j < count; j++)
    {
        buf[j] = filter_filt(f, buf[j]);
    }
}

/**
 * @brief 
 * 
 * @param f 
 */
static void low_shelf_filter_calc_coef(filter_t *f)
{
    float a0, a1, a2;
    float b0, b1, b2;
    float A, a, S_BW;
    float cosW0, sinW0;

    A = powf(10, f->g / 40);
    S_BW = f->S_BW;
    sinW0 = f->s;
    cosW0 = f->c;

    a = sinW0 * sqrtf((A + (1.0f / A)) * ((1.0f / S_BW) - 1.0f) + 2.0f);
    a0 = (A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * sqrtf(A) * a;
    a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosW0);
    a2 = (A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * sqrtf(A) * a;
    b0 = A * ((A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * sqrt(A) * a);
    b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW0);
    b2 = A * ((A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * sqrtf(A) * a);
    f->xmem1 = 0.0f;
    f->xmem2 = 0.0f;
    f->ymem1 = 0.0f;
    f->ymem2 = 0.0f;

    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
}

static void low_shelf_filter_init(filter_t *f, int frequency)
{
    f->g = 0;
    f->S_BW = 1;

    f->w0 = 2.0f * M_PI * ((float)frequency) / ((float)audio_frequency);
    f->c = cos(f->w0);
    f->s = sin(f->w0);

    f->calc_coef = low_shelf_filter_calc_coef;
}

static void high_shelf_filter_calc_coef(filter_t *f)
{
    float a0, a1, a2;
    float b0, b1, b2;
    float A, a, S_BW;
    float cosW0, sinW0;

    A = powf(10, f->g / 40);
    S_BW = f->S_BW;
    cosW0 = f->c;
    sinW0 = f->s;

    a = sinW0 * sqrtf((A + (1.0f / A)) * ((1.0f / S_BW) - 1.0f) + 2.0f);
    b0 = A * ((A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * sqrtf(A) * a);
    b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0);
    b2 = A * ((A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * sqrtf(A) * a);
    a0 = (A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * sqrtf(A) * a;
    a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0);
    a2 = (A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * sqrtf(A) * a;
    f->xmem1 = 0.0f;
    f->xmem2 = 0.0f;
    f->ymem1 = 0.0f;
    f->ymem2 = 0.0f;

    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
}

static void high_shelf_filter_init(filter_t *f, int frequency)
{
    f->g = 0;
    f->S_BW = 1;

    f->w0 = 2.0f * M_PI * ((float)frequency) / ((float)audio_frequency);
    f->c = cos(f->w0);
    f->s = sin(f->w0);

    f->calc_coef = high_shelf_filter_calc_coef;
}

/**
 * @brief calculate coefficients, of a filter, that depends only from gain
 * 
 * @param i	index of the filter whos compute coefficients
 */
static void peakingEQ_filter_calc_coef(filter_t *f)
{
    float a0, a1, a2;
    float b0, b1, b2;
    float A, a, S_BW;
    float w0, cosW0, sinW0;

    // TODO: remove me
    f->g = MAX_GAIN;
    //
    A = powf(10, f->g / 40);
    S_BW = f->S_BW;
    w0 = f->w0;
    cosW0 = f->c;
    sinW0 = f->s;

    a = sinW0 * sinhf((logf(2.0f) / 2.0f) * S_BW * w0 / sinW0);
    a0 = 1.0f + a / A;
    a1 = -2.0f * cosW0;
    a2 = 1.0f - a / A;
    b0 = 1.0f + a * A;
    b1 = -2.0f * cosW0;
    b2 = 1.0f - a * A;
    f->xmem1 = 0.0f;
    f->xmem2 = 0.0f;
    f->ymem1 = 0.0f;
    f->ymem2 = 0.0f;

    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
}

/**
 * @brief init the coefficients, of a filter, that depend only from audio_frequency
 * 
 * @param i	index of the filter whos compute coefficients
 */
static void peakingEQ_filter_init(filter_t *f, int frequency)
{
    f->g = 0;
    f->S_BW = 1;

    f->w0 = 2.0f * M_PI * ((float)frequency) / ((float)audio_frequency);
    f->c = cos(f->w0);
    f->s = sin(f->w0);

    f->calc_coef = peakingEQ_filter_calc_coef;
}

static filter_t eq_filt[NFILT]; /**< coefficients for each filter of the player. */

/**
 * @brief initialize the equalizer
 * 
 * @param frequency sampling frequency of the audio file to equalize
 */
void equalizer_init(int freq)
{
    if (freq < 0)
    {
        error_at_line(-1, 0, __FILE__, __LINE__, "audio frequency can't be negative: %d", freq);
    }
    audio_frequency = freq;
    //first filter is a low shef
    // low_shelf_filter_init(&(eq_filt[0]), equalizer_freq[0]);
    for (int i = 0; i < NFILT; i++)
    {
        peakingEQ_filter_init(&(eq_filt[i]), equalizer_freq[i]);
    }
    //last filter is an high shelf
    // high_shelf_filter_init(&eq_filt[NFILT - 1], freq);

    for (int i = 0; i < NFILT; i++)
    {
        eq_filt[i].calc_coef(&eq_filt[i]);
    }
}

/**
 * @brief equalize a stream of sample
 * 
 * @param buf buffer containing stream data
 * @param count number of sample in the buffer
 * @return int number of sample equalized, -1 on error
 */
int equalizer_equalize(float buf[], unsigned int count)
{
    if (audio_frequency < 0)
    { // equalizer init not called
        return -1;
    }
    for (int i = 0; i < NFILT; i++)
        filter_filtb(&eq_filt[i], buf, count);
    return count;
}

/**
 * @brief set the gain of a filter in the equalizer
 * 
 * @param filt index of the filter whom set the gain
 * @param gain gain value
 * @return int the new gain of the filter, -1 on error.
 */
int equalizer_set_gain(int filt, float gain)
{
    if (filt < 0 || filt > NFILT)
    {
        return -1;
    }
    if (fabs(gain) > MAX_GAIN)
    {
        return -1;
    }
    eq_filt[filt].g = gain;
    eq_filt[filt].calc_coef(&eq_filt[filt]);
    return eq_filt[filt].g;
}