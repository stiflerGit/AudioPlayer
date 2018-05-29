/**
 * @file	Player.c
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Audio Player implementation as a FSM.
 *
 * This contains the implementation of an audio player with a band
 * equalization and other functions. It is implemented as a FSM
 * where events are dispatched to event-functions, it means each
 * event as a function.
 *
 * @bug when change equalization during state != Play, it return in Play status
 */
#include "Player.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include <allegro.h>
#include <assert.h>

#define handle_error(s)	\
	do{ perror(s); exit(EXIT_FAILURE);} while(1)

#define modulus(cpx)	(sqrt(((cpx)[0] * (cpx)[0]) + ((cpx)[1] * (cpx)[1])))
#define phase(cpx)	(atan2f((cpx)[1], (cpx)[0])) 

Player	p;	/**< The player struct. */
pevent	evt;	/**< The input event. */

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
//*****************************************************************************

/**
 * @brief	Obtain the file name given its path.
 * @param[out]	name	where the file name is saved 
 * @param[in]	path	absolute/relative path of the file
 */
static void get_trackname(char *name, const char *path)
{
char	*path_;
char	*ssc;
int	l;

	path_ = path;
	ssc = strstr(path_, "/");

	do{
		l = strlen(ssc) + 1;
		path_ = &path_[strlen(path_)-l+2];
		ssc = strstr(path_, "/");
	}while(ssc);
	sprintf(name, "%s\n", path_);
}

/**
 * @brief	Load an audio file given its path.
 *
 * @param[in]	path	path of the audio file.
 * @param[out]	s	pointer to the pointer where the adress of the SAMPLE
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
int16_t	d16;	/**< buff to get the original signed value for 16bit depth. */
int8_t	d8;	/**< buff to get the original signed value for 8bit depth. */

	for (j = 0; j < count &&  j < s->len - off; j++) {
		if (s->bits == 16) {
			d16 = le16toh(((int16_t *)(s->data))[off+j]) ^ 0x8000;
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

	for (j = 0; j < count && (off + j) < s->len; j++) {
		if (s->bits == 16) {
			d16 = ((int16_t) round(buf[j])) ^ 0x8000;
			((uint16_t *) s->data)[off+j] = htole16(d16);
		} else if (s->bits == 8) {
			((uint8_t *) s->data)[off+j] = 
				((uint8_t) round(buf[j])) ^ 0x80;
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
 */
static void update_spectogram()
{
int		i;
			/**< Array index. */
int		ret;	/**< Returned values. */
float		timedata[PLAYER_WINDOW_SIZE];
			/**< Sample Timedata buff. */
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
			/**< Frequency data buff. */
static int	max = 0;
			/**< Maximum value step by step. */

	i = (pos < PLAYER_WINDOW_SIZE / 4) ? PLAYER_WINDOW_SIZE/4 : pos; 

	ret = sample_to_float((const SAMPLE *) filt_sample, timedata,
		i - PLAYER_WINDOW_SIZE/4, PLAYER_WINDOW_SIZE);
	// zero pad in case there aren't enough time data
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
	// apply blackman harris window f. to better isolate frequency
	for(i = 0; i < PLAYER_WINDOW_SIZE; i++)
		timedata[i] *= blackman_harris(i);

	FFT(timedata, freqdata);
	// calculate magnitude, euclidean distance of the real and imaginary 
	// parts and find the maximum value since last time the update_sp has 
	// been called
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		p.spectogram[i] = modulus(freqdata[i]);
		if(p.spectogram[i] > max)
			max = p.spectogram[i];
	}
	// normalize values in a [0-100] range
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		p.spectogram[i] /= max;
		// Human ear hears using a logarithmic scale.
		// This operation converts values to decibel scale.
		// Values are between [-inf, 0];
		p.spectogram[i] = 20.0f * log10f(p.spectogram[i]);
		/*
		 * Take the values we've got from the dB calculation above. 
		 * Add dynamic range value to it. Divide by that same value 
		 * and nowhave a value ranging from -infinity to 1.0f.
		 */
		p.spectogram[i] = (p.spectogram[i] + p.dynamic_range) 
			/ p.dynamic_range;
		// Clamp the lower end to 0 and you now have a range from 0 to 1
		if(p.spectogram[i] < 0) p.spectogram[i] = 0;
		// multiply that by 100 and obtain a final 0 to 100 range.
		p.spectogram[i] = (int) (p.spectogram[i] * 100);
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
	get_trackname(p.trackname, path);
	p.duration = ((float) (orig_sample->len / orig_sample->freq));
	memset(p.spectogram, 0, sizeof(p.spectogram));
	p.dynamic_range = 
		fabsf(20.0f * log10f(1.0f / (1 << orig_sample->bits)));
	printf("DYNAMIC RANGE = %f\n", p.dynamic_range);
	p.freq_spacing = ((float) orig_sample->freq) / PLAYER_WINDOW_SIZE;
	p.volume = 255;
	memcpy(p.equaliz,
		(filter[4]){
		{1, 20, 500},
		{1, 500, 2000},
		{1, 2000, 8000},
		{1, 8000, 16000}},
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
void pdispatch()
{

	if (p.state != STOP && p.state != PAUSE){
		// allegro set position = -1 when the song reached the end.
		if (voice_get_position(v) < 0) {
			PlayerStop();
		} else {
			pos = voice_get_position(v);
			p.time = (((float)pos) / ((float)orig_sample->freq));
			// Online Filtering
			PlayerFilt();
			// Spectogram update when reproducing
			update_spectogram();
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
		// convert [0-100] scale to [0-255] scale
		voice_set_volume(v, (int) (evt.val * 2.55));
		p.volume = (int) evt.val;
		break;
	case JUMP_SIG:
		// convert time to position thanks to frequency
		pos = evt.val * orig_sample->freq;
		p.time = evt.val;
		voice_set_position(v, pos);
		break;
	case FILTLOW_SIG:
		p.equaliz[0].gain = evt.val;
		break;
	case FILTMED_SIG:
		p.equaliz[1].gain = evt.val;
		break;
	case FILTMEDHIG_SIG:
		p.equaliz[2].gain = evt.val;
		break;
	case FILTHIG_SIG:
		p.equaliz[3].gain = evt.val;
		break;
	default:
		break;
	}
	evt = (pevent) {0, 0};
}

/**
 * @brief	Filter the data next to actual position
 *
 * 
 */
static void PlayerFilt()
{
int		j, k;	/**< Array indexes. */
int		mod, ph;
			/**< Module and phase of freq. bins. */
int		pos_;	/**< Need for first values. */
int		ret;	/**< Funtions return value. */
float		timedata[PLAYER_WINDOW_SIZE];
			/**< Window of data in time domain*/
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
			/**< Window of data in frequency domain*/

	pos_ = (pos < PLAYER_WINDOW_SIZE / 4) ? PLAYER_WINDOW_SIZE/4 : pos; 

	ret = sample_to_float((const SAMPLE *) orig_sample, timedata, 
		pos_ - PLAYER_WINDOW_SIZE/4, PLAYER_WINDOW_SIZE);
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
				 */
				mod = modulus(freqdata[j]) * 
					(0.0417f *  p.equaliz[k].gain + 1);
				ph = phase(freqdata[j]);
				freqdata[j][0] = mod * cosf(ph);
				freqdata[j][1] = mod * sinf(ph);
			}
		}
	}

	IFFT(freqdata, timedata);
	// copy the center of the Window
	float_to_sample((const float *) &timedata[PLAYER_WINDOW_SIZE/4], 
		filt_sample, pos_, PLAYER_WINDOW_SIZE/2);
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
		voice_set_frequency(v, orig_sample->freq);
	}
	voice_set_position(v, 0);
	memset(p.spectogram, 0, sizeof(p.spectogram));
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
		voice_set_frequency(v, orig_sample->freq);
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
		voice_set_frequency(v, orig_sample->freq);
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
		voice_set_frequency(v, (((float) orig_sample->freq) * 1.25));
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
	if (p.state != FORWARD){
		voice_set_frequency(v, (((float) orig_sample->freq) * 1.25));
	}
	//else{
	//	voice_set_frequency(v, 2 * voice_get_frequency(v));
	//}
	if (p.state == STOP || p.state == PAUSE){
		voice_start(v);
	}
	if (p.state == REWIND){
		voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_position(v, pos);
	}
	p.state = FORWARD;
}
