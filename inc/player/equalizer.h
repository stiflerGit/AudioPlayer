/**
 * @file equalizer.c
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief Audio equalizer interface. 
 * @version 0.1
 * @date 2019-03-19
 * 
 */

#ifndef EQUALIZER_H
#define EQUALIZER_H

#define EQ_NFILT 4              /**< Number of the filters of the equalizer*/
#define EQ_FILT_MAX_GAIN 20.0f  /**< Maximum deciBel gain of filters. */
#define EQ_FILT_BW 1.0f         /**< bandwidth of filters in octaves. */

extern const int equalizer_freq[EQ_NFILT]; /**< center frequency of each 
                                                band of the EQ. */

/**
 * @brief initialize the equalizer
 * 
 * @param frequency sampling frequency of the audio file to equalize
 * @return int 0 on success, -1 on fail.
 */
void equalizer_init(int frequency);

/**
 * @brief equalize a stream of sample
 * 
 * @param buf buffer containing stream data
 * @param count number of sample in the buffer
 * @return int number of sample equalized, -1 on error
 */
int equalizer_equalize(float buf[], unsigned int count);

/**
 * @brief set the gain of a filter in the equalizer
 * 
 * @param filt index of the filter whom set the gain
 * @param gain gain value
 * @return int the new gain of the filter, -1 on error.
 */
float equalizer_set_gain(int filt, float gain);

#endif //EQUALIZER_H