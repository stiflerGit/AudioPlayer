/**
 * @file	Player.c
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Audio Player implementation as a FSM.
 *
 * This contains the implementation of an audio player with a band
 * equalization and other functions. It is implemented as a FSM
 * where events are dispatched to event-functions, it means each
 * event has a function.
 * On line filtering.
 *
 * @bug 
 */
#include "Player.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libgen.h>
#include <math.h>
#include <fftw3.h>
#include <allegro.h>
#include <assert.h>

#define handle_error(s)	\
	do{ perror(s); exit(EXIT_FAILURE);} while(1)

#define modulus(cpx)	(sqrt(((cpx)[0] * (cpx)[0]) + ((cpx)[1] * (cpx)[1])))
#define phase(cpx)	(atan2f((cpx)[1], (cpx)[0])) 

Player	p;	/**< The player struct. */

static int	pos;		/**< Reproducing position. */
static int 	v;		/**< Allegro voice associated to player. */
static SAMPLE 	*orig_sample;	/**< Original Sample. Needed either for return
				to original state or to filter (equalize). */
static SAMPLE 	*filt_sample;	/**< Filtered Sample. This is the SAMPLE used 
				for sound reproduction.*/

/*******************************************************************************
 * 				Player Events
 ******************************************************************************/
static void PlayerFilt();
static void PlayerPlay();
static void PlayerPause();
static void PlayerStop();
static void PlayerRewind();
static void PlayerForward();
/******************************************************************************/

/**
 * @brief	Obtain the file name given its path.
 * @param[out]	name	where the file name is saved 
 * @param[in]	path	absolute/relative path of the file
 */
static void get_trackname(char name[], const char path[])
{
char	path_[1024];
char	*name_;

	strcpy(path_, path);
	name_ = basename(path_);

	sprintf(name, "%s\n", name_);
	name[strlen(name)-1] = '\0';
}

/**
 * @brief	Load an audio file given its path.
 *
 * @param[in]	path	path of the audio file.
 * @param[out]	s	pointer to the pointer where the address of the SAMPLE
 * 			struct returned by allegro lib is saved.
 */
static void algr_load_smpl(const char * path, SAMPLE **s)
{
char	pass;		/**< Audio file controls result. */
char	err[1024];	/**< Error string. */

	pass = 1;
	*s = load_sample(path);
	if (!(*s))
		handle_error("load_sample");

	if ((*s)->bits > PLAYER_MAX_SMPL_SIZE * 8) {
		strcpy(err, "sample too big\n");
		pass = 0;
	}
	if ((*s)->stereo >= PLAYER_MAX_NCH) {
		strcat(err, "too much channels\n");
		pass = 0;
	}
	if ((*s)->freq > PLAYER_MAX_FREQ) {
		strcat(err, "too much frequency per seconds\n");
		pass = 0;
	}
	if (pass == 0) {
		destroy_sample(*s);
		printf("%s\n", err);
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief	Transform an Allegro SAMPLE piece of data to a machine float 
 *		stream.
 *
 * 
 * The sample data are always in unsigned format. This means that would have 
 * to XOR every sample value with 0x8000 to change the signedness. 
 * Unfortunately allegro supports only 8 and 16 bits depth wav.
 * @ref		https://liballeg.org/stabledocs/en/alleg001.html#SAMPLE
 *
 * @param[in]	s	address of the Allegro SAMPLE struct.
 * @param[out]	buf	adress of the float stream.
 * @param[in]	off	offset by which start to convert.
 * @param[in]	count	no. data to convert.
 * @ret			no. samples converted
 */
static int sample_to_float(const SAMPLE *s, float *buf, unsigned int off, 
				unsigned int count)
{
int	j;	/**< sample data index */
int32_t d32;	/**< buff to get the original signed value for 32bit depth. */
int16_t	d16;	/**< buff to get the original signed value for 16bit depth. */
int8_t	d8;	/**< buff to get the original signed value for 8bit depth. */

	for (j = 0; j < count &&  j < s->len - off; j++) {
		if (s->bits == 32) {
			d32 = le32toh(((uint32_t *)(s->data))[off+j]) ^ 0x80000000;
			buf[j] = (float) d32;
		} else if (s->bits == 16) {
			d16 = le16toh(((uint16_t *)(s->data))[off+j]) ^ 0x8000;
			buf[j] = (float) d16;
		} else if (s->bits == 8) {
			d8 = ((uint8_t *) (s->data))[off+j] ^ 0x80;
			buf[j] = (float) d8;
		}
	}
	return j;
}

/**
 * @brief	Transform a machine float stream to an Allegro SAMPLE piece of 
 * 		data.
 * @param[in]	buf	adress of the float stream.
 * @param[out]	s	address of the Allegro SAMPLE struct.
 * @param[in]	off	offset by which start to convert.
 * @param[in]	count	no. data to convert.
 * @ret			no. data converted.
 */
static int float_to_sample(const float *buf, SAMPLE *s, int off, int count)
{
int		j;	/**< sample data index */
uint16_t	d16;	/**< buff to get the original signed value. */
uint32_t	d32;	/**< buff to get the original signed value. */

	for (j = 0; j < count && (off + j) < s->len; j++) {
		if (s->bits == 32) {
			d32 = (int32_t)(round(buf[j])) ^ 0x80000000;
			((uint32_t *) s->data)[off+j] = htole32(d32);
		} else if (s->bits == 16) {
			d16 = (int16_t)(round(buf[j])) ^ 0x8000,
			((uint16_t *) s->data)[off+j] = htole16(d16);
			/*
			// float could be too high to be represented on a uint16
			if (fabs(buf[j]) <= 0x7FFF){
				d16 = (int16_t)(round(buf[j])) ^ 0x8000,
				((uint16_t *) s->data)[off+j] = htole16(d16);
			} else {
				// Do nothing in case and notice to the user
				printf("float sample too high\n");
			}
			*/
		} else if (s->bits == 8) {
			((uint8_t *) s->data)[off+j] = 
				((uint8_t) round(buf[j])) ^ 0x80;
			/*
			// float could be too high to be represented on a uint8
			if (fabs(buf[j]) <= 0x7E){
				((uint8_t *) s->data)[off+j] = 
					((int8_t) round(buf[j])) ^ 0x80;
			} else {
				// Do nothing in case and notice to the user
				printf("float sample too high\n");
			}
			*/
		}
	}
	return j;
}

/**
 * @brief 	Blackman-Harris: window function that provides a far better
 *		frequency isolation in the frequency domain.
 * @param[in]	n	no. the sample in the window.
 * @ret			the value computed.
 */
static float blackman_harris(int n)
{
const float	a0 = 0.35875f;
const float	a1 = 0.48829f;
const float	a2 = 0.14128f;
const float	a3 = 0.01168f;
float		wn;

	wn = (float) (a0 - a1 * cos((2 * M_PI * n) / (PLAYER_WINDOW_SIZE -1))
		+ a2 * cos((4 * M_PI * n) / (PLAYER_WINDOW_SIZE -1))
		- a3 * cos((6 * M_PI * n) / (PLAYER_WINDOW_SIZE -1)));

	return wn;
}

/**
 * @brief	Compute the Fast Fourier Transrmation of a fixed size float 
 *		buffer. It means only real data in input.
 * @param[in]	input float buffer. Size: PLAYER_WINDOW_SIZE.
 * @param[out]	buffer composed by complex data. Size: PLAYER_WINDOW_SIZE / 2;
 */
static void FFT(const float *in, fftwf_complex *out)
{
fftwf_plan	dft_p;				/**< Direct f.t. configuration 
						structure. */
float		inbuff[PLAYER_WINDOW_SIZE];	/**< Need a buffer, cause create
						plan destroy input data, */

	dft_p = fftwf_plan_dft_r2c_1d(PLAYER_WINDOW_SIZE, inbuff, out,
			FFTW_ESTIMATE);
	memcpy(inbuff, in, sizeof(inbuff));
	fftwf_execute(dft_p);
	fftwf_destroy_plan(dft_p);
}

/**
 * @brief	Compute the Inverse Fast Fourier Transrmation of a fixed size
 *		complex data buffer.
 * @param[in]	input complex data buffer. Size: PLAYER_WINDOW_SIZE / 2.
 * @param[out]	float buffer. Size: PLAYER_WINDOW_SIZE;
 */
static void IFFT(const fftwf_complex *in, float *out)
{
int		i;
fftwf_plan	ift_p;				/**< Inverse f.t. configuration
						structure. */
fftwf_complex	inbuff[PLAYER_WINDOW_SIZE_CPX];	/**< Need a buffer, cause create 
						plan destroy input data, */

	ift_p = fftwf_plan_dft_c2r_1d(PLAYER_WINDOW_SIZE, inbuff, out,
			FFTW_ESTIMATE);
	memcpy(inbuff, in, sizeof(inbuff));
	fftwf_execute(ift_p);
	for (i = 0; i < PLAYER_WINDOW_SIZE; i++) {
		out[i] /= PLAYER_WINDOW_SIZE;
	}
	fftwf_destroy_plan(ift_p);
}

/**
 * @brief	Update the player spectogram according to the current playing
 *		position. 
 *
 * For a good spectogram the timedata Window is first passed through the black-
 * -man harris Window function, which better isolate frequency. After that com-
 * -pute the FFT and than magnitude (euclidean distance of the real and imagina-
 * -ry) parts for each term. In latter operation the maximum value among all 
 * bins is computed as well.
 * In order to provide a spectogram easy to visualize and understand the func-
 * -tion normalizes the bins, that are actually random positive values.
 * Normalization comes with first dividing all bins for maximum value computed
 * before. Now bins are in the [0-1] range, but human ear hears using a loga-
 * -rithmic scale (deciBel scale). So bins are now passed throgh a logaritmig 
 * function, which brings the values between [-inf, 0]. 
 * Take the values we've got from the dB calculation above. Add dynamic range 
 * value to it. Divide by that same value  and nowhave a value ranging from
 * -infinity to 1. This operation is done to bring values to the original sample
 * bit depth scale. Finally clamping the lower end to 0 and multiplying by 100
 * the bins are in the [0-100] scale.
 *
 * @param[in]	s
 * @param[out]	spect
 * @param[in]	pos
 */
static void update_spectogram(const SAMPLE *s, float spect[])
{
int		i;	/**< Array index. */
int		ret;	/**< Returned values. */
float		timedata[PLAYER_WINDOW_SIZE];
			/**< Sample Timedata buff. */
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
			/**< Frequency data buff. */
static int	max = 0;
			/**< Maximum value step by step. */

	i = (pos < PLAYER_WINDOW_SIZE / 4) ? PLAYER_WINDOW_SIZE/4 : pos; 

	ret = sample_to_float((const SAMPLE *) s, timedata,
		i - PLAYER_WINDOW_SIZE/4, PLAYER_WINDOW_SIZE);
	// Zero pad in case there aren't enough time data
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
	// Apply blackman harris window f. to better isolate frequency
	for(i = 0; i < PLAYER_WINDOW_SIZE; i++)
		timedata[i] *= blackman_harris(i);

	FFT(timedata, freqdata);
	// Magnitude, maximum value 
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		spect[i] = modulus(freqdata[i]);
		if(spect[i] > max)
			max = spect[i];
	}
	// Normalize values in a [0-100] range
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		spect[i] /= max;
		// Human ear hears using a logarithmic scale.
		spect[i] = 20.0f * log10f(spect[i]);
		// Bring values to the original bit depth scale.
		spect[i] = (spect[i] + p.dynamic_range) 
			/ p.dynamic_range;
		// Clamp the lower end to 0 and you now have a range from 0 to 1
		if(spect[i] < 0) spect[i] = 0;
		// Multiply that by 100 and obtain a final 0 to 100 range.
		spect[i] = (int) (spect[i] * 100);
	}
}

/**
 * @brief	initialize the player internal and external variable.
 * @param[in]	path	path of the input song.
 */
void pinit(const char *path)
{
	algr_load_smpl(path, &orig_sample);
	algr_load_smpl(path, &filt_sample);

	p.state = STOP;
	p.time = pos = 0;
	p.time_data = 0;
	p.bits = filt_sample->bits;
	get_trackname(p.trackname, path);
	p.duration = ((float) (filt_sample->len / filt_sample->freq));
	memset(p.filt_spect, 0, sizeof(p.filt_spect));
	memset(p.orig_spect, 0, sizeof(p.orig_spect));
	p.dynamic_range = 
		fabsf(20.0f * log10f(1.0f / (1 << filt_sample->bits)));
	printf("DYNAMIC RANGE = %f\n", p.dynamic_range);
	p.freq_spacing = ((float) filt_sample->freq) / PLAYER_WINDOW_SIZE;
	p.volume = 255;
	memcpy(p.equaliz,
		(filter[4]){
		{0, 1, 250},		// sub bass, bass
		{0, 251, 2000},		// lower midrange, midrange
		{0, 2001, 4000},	// upper midrange
		{0, 4001, 20000}},	// presence and brilliance
		sizeof(filter[4]));
	// allocating the sample
	v = allocate_voice(filt_sample);
	if (v < 0)
		handle_error("allocate_voice");
	voice_set_playmode(v, PLAYMODE_PLAY);
}

/**
 * @brief	take the external global event and call the function
 *		corresponding to the right funtion. It also cleans the
 * 		event variable each time is called.
 */
void pdispatch(pevent evt)
{

	if (p.state != STOP && p.state != PAUSE){
		// allegro set position = -1 when the song reached the end.
		if (voice_get_position(v) < 0) {
			PlayerStop();
		} else {
			pos = voice_get_position(v);
			p.time = (((float)pos) / ((float)filt_sample->freq));
			// Online Filtering
			PlayerFilt();
			// Spectogram update when reproducing
			sample_to_float(filt_sample, &p.time_data, pos, 1); 
			update_spectogram(orig_sample, p.orig_spect);
			update_spectogram(filt_sample, p.filt_spect);
		}
	}
	switch (evt.sig) {
	case STOP_SIG:
		PlayerStop();
		break;
	case PLAY_SIG:
		PlayerPlay();
		break;
	case PAUSE_SIG:
		PlayerPause();
		break;
	case RWND_SIG:
		PlayerRewind();
		break;
	case FRWD_SIG:
		PlayerForward();
		break;
	case VOL_SIG:
		if (evt.val > 100) evt.val = 100;
		if (evt.val < 0) evt.val = 0;
		// convert [0-100] scale to [0-255] scale
		voice_set_volume(v, (int) (evt.val * 2.55));
		p.volume = (int) evt.val;
		break;
	case JUMP_SIG:
		if (evt.val > p.duration) evt.val = p.duration;
		if (evt.val < 0) evt.val = 0;
		// convert time to position thanks to frequency
		pos = evt.val * filt_sample->freq;
		p.time = evt.val;
		voice_set_position(v, pos);
		break;
	case FILTLOW_SIG:
		if (evt.val > MAX_GAIN) evt.val = MAX_GAIN;
		if (evt.val < MIN_GAIN) evt.val = MIN_GAIN;
		p.equaliz[0].gain = evt.val;
		break;
	case FILTMED_SIG:
		if (evt.val > MAX_GAIN) evt.val = MAX_GAIN;
		if (evt.val < MIN_GAIN) evt.val = MIN_GAIN;
		p.equaliz[1].gain = evt.val;
		break;
	case FILTMEDHIG_SIG:
		if (evt.val > MAX_GAIN) evt.val = MAX_GAIN;
		if (evt.val < MIN_GAIN) evt.val = MIN_GAIN;
		p.equaliz[2].gain = evt.val;
		break;
	case FILTHIG_SIG:
		if (evt.val > MAX_GAIN) evt.val = MAX_GAIN;
		if (evt.val < MIN_GAIN) evt.val = MIN_GAIN;
		p.equaliz[3].gain = evt.val;
		break;
	default:
		break;
	}
}

/**
 * @brief	Filter the data nexts to actual position
 *
 * 
 */
static void PlayerFilt()
{
int		j, k;	/**< Array indexes. */
float		mod, ph;
			/**< Module and phase of freq. bins. */
int		pos_;	/**< Need for first values. */
int		ret;	/**< Funtions return value. */
float		timedata[PLAYER_WINDOW_SIZE];
			/**< Window of data in time domain*/
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
			/**< Window of data in frequency domain*/

	pos_ = (pos < PLAYER_WINDOW_SIZE / 4) ? PLAYER_WINDOW_SIZE/4 : pos; 

	ret = sample_to_float((const SAMPLE *) orig_sample, timedata, 
		pos_ + PLAYER_WINDOW_SIZE / 2, PLAYER_WINDOW_SIZE);
	// zero pad in case there aren't enough time data
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);

	FFT(timedata, freqdata);
	// filt data in the frequency domain
	for (j = 0; j < PLAYER_WINDOW_SIZE_CPX; j++) {
		for (k = 0; k < PLAYER_NFILT; k++) {
			if ((j * p.freq_spacing) > p.equaliz[k].low_bnd
			&& (j * p.freq_spacing) < p.equaliz[k].upp_bnd) {
				/**
				 * TO-DO:	the gain is given in dB
				 *		I have to find the link beetwen
				 *		dB gain and the multiplication 
				 *		here;
				 * DONE: 	MOD = 10 ^ (DB - 20)
				 */
				mod = 20.0f * log10f(modulus(freqdata[j]));
				mod += p.equaliz[k].gain;
				mod = pow(10, mod/20);
				ph = phase(freqdata[j]);
				freqdata[j][0] = mod * cosf(ph);
				freqdata[j][1] = mod * sinf(ph);
			}
		}
	}

	IFFT(freqdata, timedata);
	// copy the center of the Window
	float_to_sample((const float *) &timedata[PLAYER_WINDOW_SIZE/4], 
		filt_sample, pos_ + PLAYER_WINDOW_SIZE, PLAYER_WINDOW_SIZE/2);
}

/**
 * @brief	Function that manage the STOP_SIG event.
 *		It stop to reproduce the sound.
 * 		It brings the state to STOP, player position and time to 0,
 *		and reset all the spectogram to 0.
 */
static void PlayerStop()
{
	if(p.state != PAUSE)
		voice_stop(v);
	if (p.state == REWIND || p.state == FORWARD) {
		if (p.state == REWIND)
			voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_frequency(v, filt_sample->freq);
	}
	voice_set_position(v, 0);
	memset(p.filt_spect, 0, sizeof(p.filt_spect));
	memset(p.orig_spect, 0, sizeof(p.orig_spect));
	p.time = pos = 0;
	p.state = STOP;
}

/**
 * @brief	Function that manage the PLAY_SIG event.
 */
static void PlayerPlay()
{
	if (p.state == STOP || p.state == PAUSE)
		voice_start(v);
	if (p.state == REWIND || p.state == FORWARD) {
		if (p.state == REWIND){
			voice_set_playmode(v, PLAYMODE_FORWARD);
			voice_set_position(v, pos);
		}
		// set frequency to the original freq.
		voice_set_frequency(v, filt_sample->freq);
	}
	p.state = PLAY;
}

/**
 * @brief	Function that manage the PAUSE_SIG event.
 */
static void PlayerPause()
{
	if(p.state != STOP && p.state != PAUSE){
		voice_stop(v);
	}
	if (p.state == REWIND || p.state == FORWARD){
		voice_set_frequency(v, filt_sample->freq);
		if(p.state == REWIND){
			voice_set_playmode(v, PLAYMODE_FORWARD);
			voice_set_position(v, pos);
		}
	}
	p.state = PAUSE;
}

/**
 * @brief	Function that manage the RWND_SIG event.
 */
static void PlayerRewind()
{
	if(p.state != REWIND && p.state != STOP){
		voice_set_playmode(v, PLAYMODE_BACKWARD);
		voice_set_position(v, pos);
		voice_set_frequency(v, (((float) filt_sample->freq) * 1.25));
	}
	else {
		voice_set_frequency(v, 1.25 *  voice_get_frequency(v));
	}
	//else if (p.tate == REWIND)
	//	voice_set_frequency(v, 2 * voice_get_frequency(v));
	if (p.state == PAUSE)
		voice_start(v);
	p.state = REWIND;
}

/**
 * @brief	Function that manage the FRWD_SIG event.
 */
static void PlayerForward()
{
	if (p.state == REWIND){
		voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_position(v, pos);
		voice_set_frequency(v, (((float) filt_sample->freq) * 1.25));
	}
	else {
		voice_set_frequency(v, 1.25 * voice_get_frequency(v));
	}
	if (p.state == STOP || p.state == PAUSE){
		voice_start(v);
	}
	p.state = FORWARD;
}
