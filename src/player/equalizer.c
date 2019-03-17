#include "player/equalizer.h"

#include <math.h>

static int audio_frequency;

const int equalizer_freq[NFILT] = {250, 2000, 5000, 10000};

/**
 * @brief contains all coefficients needed to filter a sample.
 * 
 * Coeficient needed for filter a stream of time-data audio signal.
 * As explained in:
 * @ref https://www.w3.org/2011/audio/audio-eq-cookbook.html
 */
typedef struct
{
    float g;
    float A;
    float w0;
    float s, c;
    float alpha;
    float a1, a2;
    float b0, b1, b2;
    double xmem1, xmem2;
    double ymem1, ymem2;
} filtert_t;

filtert_t eq_filt[NFILT]; /**< coefficients for each filter of the player. */

/**
 * @brief init the coefficients, of a filter, that depend only from audio_frequency
 * 
 * @param i	index of the filter whos compute coefficients
 */
static void filter_init(int i)
{
    eq_filt[i].w0 = 2 * M_PI * equalizer_freq[i] / audio_frequency;
    eq_filt[i].c = cos(eq_filt[i].w0);
    eq_filt[i].s = sin(eq_filt[i].w0);
    eq_filt[i].alpha = eq_filt[i].s * sinh(log(2) / 2 *
                                           eq_filt[i].w0 / eq_filt[i].s);
}

/**
 * @brief calculate coefficients, of a filter, that depends only from gain
 * 
 * @param i	index of the filter whos compute coefficients
 */
static void filter_calc_coef(int i)
{
    float a0;

    eq_filt[i].A = pow(10, eq_filt[i].g / 40);
    a0 = 1 + eq_filt[i].alpha / eq_filt[i].A;
    eq_filt[i].a1 = (-2 * eq_filt[i].c) / a0;
    eq_filt[i].a2 = (1 - eq_filt[i].alpha / eq_filt[i].A) / a0;
    eq_filt[i].b0 = (1 + eq_filt[i].alpha * eq_filt[i].A) / a0;
    eq_filt[i].b1 = (-2 * eq_filt[i].c) / a0;
    eq_filt[i].b2 = (1 - eq_filt[i].alpha * eq_filt[i].A) / a0;
    eq_filt[i].xmem1 = 0;
    eq_filt[i].xmem2 = 0;
    eq_filt[i].ymem1 = 0;
    eq_filt[i].ymem2 = 0;
}

/**
 * @brief filter a single sample if time data
 * 
 * @param[in] i 	index of the filter to use for filtering
 * @param[in] x 	sample of time data
 * @return float 	resulting filtered sample
 */
static float filter_filt(int i, float x)
{
    double y;

    y = eq_filt[i].b0 * x +
        eq_filt[i].b1 * eq_filt[i].xmem1 +
        eq_filt[i].b2 * eq_filt[i].xmem2 -
        eq_filt[i].a1 * eq_filt[i].ymem1 -
        eq_filt[i].a2 * eq_filt[i].ymem2;

    eq_filt[i].xmem2 = eq_filt[i].xmem1;
    eq_filt[i].xmem1 = x;
    eq_filt[i].ymem2 = eq_filt[i].ymem1;
    eq_filt[i].ymem1 = y;

    return y;
}

/**
 * @brief filter a buffer(stream) of time data
 * 
 * @param[in] i			index of the filter to use for filtering
 * @param[inout] buf	time-data samples 
 * @param count 		number of samples in buf
 */
static void filter_filtb(int i, float buf[], unsigned int count)
{
    int j;

    for (j = 0; j < count; j++)
    {
        buf[j] = filter_filt(i, buf[j]);
    }
}
/******************************************************************************/

int equalizer_init(int freq)
{
    if (freq < 0)
    {
        return -1;
    }
    audio_frequency = freq;
    for (int i = 0; i < NFILT; i++)
    {
        filter_init(i);
        eq_filt[i].g = 0;
        filter_calc_coef(i);
    }
    return 0;
}

int equalizer_equalize(float buf[], unsigned int count)
{
    if (audio_frequency < 0)
    { // equalizer init not called
        return -1;
    }
    for (int i = 0; i < NFILT; i++)
        filter_filtb(i, buf, count);
    return count;
}

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
    filter_calc_coef(filt);
    return eq_filt[filt].g;
}