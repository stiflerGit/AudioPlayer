#ifndef EQUALIZER_H
#define EQUALIZER_H

#define NFILT 4 /**< Number of the filters of the equalizer*/

// TODO: remove ifndef
#ifndef MAX_GAIN
#define MAX_GAIN 20 /**< Maximum Decibel gain of filters. */
#endif

extern const int equalizer_freq[NFILT]; /**< center frequency of each band of the EQ. */

/**
 * @brief 
 * 
 * @param frequency 
 * @return int 
 */
int equalizer_init(int frequency);

/**
 * @brief 
 * 
 * @param buf 
 * @param count 
 * @return int 
 */
int equalizer_equalize(float buf[], unsigned int count);

/**
 * @brief 
 * 
 * @param filt 
 * @param gain 
 * @return int 
 */
int equalizer_set_gain(int filt, float gain);

#endif //EQUALIZER_H