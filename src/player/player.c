/**
 * @file	player.c
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Audio Player implementation as a FSM.
 *
 * This contains the implementation of an audio player.
 * It is implemented as a FSM, it's step is executed every given
 * period by a thread. Events are dispatched to event-functions, 
 * it means each event has a function.
 *
 */
#include "player/player.h"
// standards libraries
#include <stdio.h>
#include <stdint.h>
//
#include <assert.h>
#include <error.h>
#include <libgen.h>
#include <math.h>
#include <string.h>
//
#include <fftw3.h>
#include <allegro.h>

#include "defines.h"
#include "player/equalizer.h"
#include "ptask.h"

#define modulus(cpx) (sqrt(((cpx)[0] * (cpx)[0]) + ((cpx)[1] * (cpx)[1])))
#define phase(cpx) (atan2f((cpx)[1], (cpx)[0]))

Player_t p; /**< The player struct. */

static int pos;				/**< Reproducing position. */
static int filt_pos = 0;	/**< Filtering position. */
static int v;				/**< Allegro voice associated to player. */
static SAMPLE *orig_sample; /**< Original Sample. Needed either for return
				to original state or to filter (equalize). */
static SAMPLE *filt_sample; /**< Filtered Sample. This is the SAMPLE used 
				for sound reproduction.*/

static task_par_t tp = {
	arg : 0,
	period : 80,
	deadline : 80,
	priority : 20,
	dmiss : 0,
}; /**< default task parameters. */

static pthread_t tid; /**< player thread identifier. */

static player_event_t player_event;		   /**< event for dispatch. */
static pthread_mutex_t player_event_mutex; /**< mutex for the event. */

static char _player_exit = 0; /**< variable to notice the thread that has to exit. */

/**
 * @brief player thread routine
 * 
 * @param[in] arg  argument passed to the routine(actually nothing is passed)
 * @return void* pointer to the variable returned by the thread(actually nothing)
 */
static void *player_run(void *arg);

/*******************************************************************************
 * 				Player Events
 ******************************************************************************/
static void player_filt();
static void player_play();
static void player_pause();
static void player_stop();
static void player_rewind();
static void player_forward();
/******************************************************************************/

/**
 * @brief	Obtain the file name given its path.
 * @param[out]	name	where the file name is saved 
 * @param[in]	path	absolute/relative path of the file
 */
static void get_trackname(char name[], const char path[])
{
	char path_[1024];
	char *name_;

	strcpy(path_, path);
	name_ = basename(path_);

	sprintf(name, "%s\n", name_);
	name[strlen(name) - 1] = '\0';
}

/**
 * @brief	Load an audio file given its path.
 *
 * @param[in]	path	path of the audio file.
 * @param[out]	s	pointer to the pointer where the address of the SAMPLE
 * 			struct returned by allegro lib is saved.
 */
static void algr_load_smpl(const char *path, SAMPLE **s)
{
	char pass;		/**< Audio file controls result. */
	char err[1024]; /**< Error string. */

	pass = 1;
	*s = load_sample(path);
	if (!(*s))
		error_at_line(-1, ENOENT, __FILE__, __LINE__, "%s", path);

	if ((*s)->bits > PLAYER_MAX_SMPL_SIZE * 8)
	{
		strcpy(err, "sample too big, ");
		pass = 0;
	}
	if ((*s)->stereo >= PLAYER_MAX_NCH)
	{
		strcat(err, "too much channels, ");
		pass = 0;
	}
	if ((*s)->freq > PLAYER_MAX_FREQ)
	{
		strcat(err, "too much frequency per seconds");
		pass = 0;
	}
	if (pass == 0)
	{
		destroy_sample(*s);
		error_at_line(-1, 0, __FILE__, __LINE__, "%s", err);
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
 * @return			no. samples converted
 */
static int sample_to_float(const SAMPLE *s, float *buf, unsigned int off,
						   unsigned int count)
{
	int j;		 /**< sample data index */
	int16_t d16; /**< buff to get the original signed value for 16bit depth. */
	int8_t d8;   /**< buff to get the original signed value for 8bit depth. */

	for (j = 0; j < count && j < s->len - off; j++)
	{
		if (s->bits == 16)
		{
			d16 = le16toh(((uint16_t *)(s->data))[off + j]) ^ 0x8000;
			buf[j] = (float)d16;
		}
		else if (s->bits == 8)
		{
			d8 = ((uint8_t *)(s->data))[off + j] ^ 0x80;
			buf[j] = (float)d8;
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
 * @return			no. data converted.
 */
static int float_to_sample(const float *buf, SAMPLE *s, int off, int count)
{
	int j;		  /**< sample data index */
	uint16_t d16; /**< buff to get the original signed value. */

	for (j = 0; j < count && (off + j) < s->len; j++)
	{
		if (s->bits == 16)
		{
			d16 = (int16_t)(round(buf[j])) ^ 0x8000,
			((uint16_t *)s->data)[off + j] = htole16(d16);
		}
		else if (s->bits == 8)
		{
			((uint8_t *)s->data)[off + j] =
				((uint8_t)round(buf[j])) ^ 0x80;
		}
	}
	return j;
}

/**
 * @brief 	Blackman-Harris: window function that provides a far better
 *		frequency isolation in the frequency domain.
 * @param[in]	n	no. the sample in the window.
 * @return			the value computed.
 */
static float blackman_harris(int n)
{
	const float a0 = 0.35875f;
	const float a1 = 0.48829f;
	const float a2 = 0.14128f;
	const float a3 = 0.01168f;
	float wn;

	wn = (float)(a0 - a1 * cos((2 * M_PI * n) / (PLAYER_WINDOW_SIZE - 1)) + a2 * cos((4 * M_PI * n) / (PLAYER_WINDOW_SIZE - 1)) - a3 * cos((6 * M_PI * n) / (PLAYER_WINDOW_SIZE - 1)));

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
	fftwf_plan dft_p; /**< Direct f.t. configuration structure. */
	float inbuff[PLAYER_WINDOW_SIZE];
	/**< Need a buffer, cause create plan destroy input data, */

	dft_p = fftwf_plan_dft_r2c_1d(PLAYER_WINDOW_SIZE, inbuff, out,
								  FFTW_ESTIMATE);
	memcpy(inbuff, in, sizeof(inbuff));
	fftwf_execute(dft_p);
	fftwf_destroy_plan(dft_p);
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
	int i;   /**< Array index. */
	int ret; /**< Returned values. */
	float timedata[PLAYER_WINDOW_SIZE];
	/**< Sample Timedata buff. */
	fftwf_complex freqdata[PLAYER_WINDOW_SIZE_CPX];
	/**< Frequency data buff. */
	static int max = 0;
	/**< Maximum value step by step. */

	i = (pos < PLAYER_WINDOW_SIZE / 4) ? PLAYER_WINDOW_SIZE / 4 : pos;

	ret = sample_to_float((const SAMPLE *)s, timedata,
						  i - PLAYER_WINDOW_SIZE / 4, PLAYER_WINDOW_SIZE);
	// Zero pad in case there aren't enough time data
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
	// Apply blackman harris window f. to better isolate frequency
	for (i = 0; i < PLAYER_WINDOW_SIZE; i++)
		timedata[i] *= blackman_harris(i);

	FFT(timedata, freqdata);
	// Magnitude, maximum value
	for (i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++)
	{
		spect[i] = modulus(freqdata[i]);
		if (spect[i] > max)
			max = spect[i];
	}
	// Normalize values in a [0-100] range
	for (i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++)
	{
		spect[i] /= max;
		// Human ear hears using a logarithmic scale.
		spect[i] = 20.0f * log10f(spect[i]);
		// Bring values to the original bit depth scale.
		spect[i] = (spect[i] + p.dynamic_range) / p.dynamic_range;
		// Clamp the lower end to 0 and you now have a range from 0 to 1
		if (spect[i] < 0)
			spect[i] = 0;
		// Multiply that by 100 and obtain a final 0 to 100 range.
		spect[i] = (int)(spect[i] * 100);
	}
}

/**
 * @brief	initialize the player internal and external variable.
 * @param[in]	path	path of the input song.
 */
void player_init(const char *path)
{
	algr_load_smpl(path, &orig_sample);
	algr_load_smpl(path, &filt_sample);

	p.state = STOP;
	p.time = pos = 0;
	p.time_data = 0;
	p.bits = filt_sample->bits;
	get_trackname(p.trackname, path);
	p.duration = ((float)(filt_sample->len / filt_sample->freq));
	memset(p.filt_spect, 0, sizeof(p.filt_spect));
	memset(p.orig_spect, 0, sizeof(p.orig_spect));
	p.dynamic_range =
		fabsf(20.0f * log10f(1.0f / (1 << filt_sample->bits)));
	p.freq_spacing = ((float)filt_sample->freq) / PLAYER_WINDOW_SIZE;
	p.volume = 255;
	// initialize of Band EQ.
	memset(p.eq_gain, 0, sizeof(p.eq_gain));
	equalizer_init(filt_sample->freq);
	// allocating the sample
	v = allocate_voice(filt_sample);
	if (v < 0)
		error_at_line(-1, 0, __FILE__, __LINE__, "no voices are available");
	voice_set_playmode(v, PLAYMODE_PLAY);
}

/**
 * @brief call the function corresponding to the given event
 * 
 * This is called by the thread when external player dispatch
 * interface changed the value of the global variable event
 * 
 * @param evt event to dispatch
 */
static void player_dispatch_body(player_event_t evt)
{
	int ret;
	switch (evt.sig)
	{
	case STOP_SIG:
		player_stop();
		break;
	case PLAY_SIG:
		player_play();
		break;
	case PAUSE_SIG:
		player_pause();
		break;
	case RWND_SIG:
		player_rewind();
		break;
	case FRWD_SIG:
		player_forward();
		break;
	case VOL_SIG:
		if (evt.val > 100)
			evt.val = 100;
		if (evt.val < 0)
			evt.val = 0;
		// convert [0-100] scale to [0-255] scale
		voice_set_volume(v, (int)(evt.val * 2.55));
		p.volume = (int)evt.val;
		break;
	case JUMP_SIG:
		if (evt.val > p.duration)
			evt.val = p.duration;
		if (evt.val < 0)
			evt.val = 0;
		// convert time to position thanks to frequency
		pos = evt.val * filt_sample->freq;
		p.time = evt.val;
		voice_set_position(v, pos);
		break;
	case FILTLOW_SIG:
	case FILTMED_SIG:
	case FILTMEDHIG_SIG:
	case FILTHIG_SIG:
		if (abs(evt.val) > MAX_GAIN)
		{
			evt.val = (evt.val < 0) ? -MAX_GAIN : MAX_GAIN;
		}
		// TODO: control on evt.val and ret value for error detection
		ret = equalizer_set_gain(evt.sig - FILTLOW_SIG, evt.val);
		p.eq_gain[evt.sig - FILTLOW_SIG] = ret;
		filt_pos = pos;
		break;
	default:
		break;
	}
}

/**
 * @brief	Filter the data nexts to actual position
 */
static void player_filt()
{
	int ret;
	float timedata[PLAYER_MAX_FREQ / 2];
	/**< half second window(in the worst case frequency) 
			of time data*/

	if (filt_pos < filt_sample->len)
	{
		ret = sample_to_float((const SAMPLE *)orig_sample,
							  timedata, filt_pos, PLAYER_MAX_FREQ / 2);
		ret = equalizer_equalize(timedata, ret);
		float_to_sample(timedata, filt_sample, filt_pos, ret);
		filt_pos += PLAYER_MAX_FREQ / 2;
	}
}

/**
 * @brief	Function that manage the STOP_SIG event.
 *		
 * It stop to reproduce the sound.
 * It brings the state to STOP, player position and time to 0,
 * and reset all the spectogram to 0.
 */
static void player_stop()
{
	if (p.state != PAUSE)
		voice_stop(v);
	if (p.state == REWIND || p.state == FORWARD)
	{
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
static void player_play()
{
	if (p.state == STOP || p.state == PAUSE)
		voice_start(v);
	if (p.state == REWIND || p.state == FORWARD)
	{
		if (p.state == REWIND)
		{
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
static void player_pause()
{
	if (p.state != STOP && p.state != PAUSE)
	{
		voice_stop(v);
	}
	if (p.state == REWIND || p.state == FORWARD)
	{
		voice_set_frequency(v, filt_sample->freq);
		if (p.state == REWIND)
		{
			voice_set_playmode(v, PLAYMODE_FORWARD);
			voice_set_position(v, pos);
		}
	}
	p.state = PAUSE;
}

/**
 * @brief	Function that manage the RWND_SIG event.
 */
static void player_rewind()
{
	if (p.state != REWIND && p.state != STOP)
	{
		voice_set_playmode(v, PLAYMODE_BACKWARD);
		voice_set_position(v, pos);
		voice_set_frequency(v, (((float)filt_sample->freq) * 1.25));
	}
	else
	{
		voice_set_frequency(v, 1.25 * voice_get_frequency(v));
	}
	if (p.state == PAUSE)
		voice_start(v);
	p.state = REWIND;
}

/**
 * @brief	Function that manage the FRWD_SIG event.
 */
static void player_forward()
{
	if (p.state == REWIND)
	{
		voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_position(v, pos);
		voice_set_frequency(v, (((float)filt_sample->freq) * 1.25));
	}
	else
	{
		voice_set_frequency(v, 1.25 * voice_get_frequency(v));
	}
	if (p.state == STOP || p.state == PAUSE)
	{
		voice_start(v);
	}
	p.state = FORWARD;
}

/**
 * @brief player destructor
 * 
 */
void player_xtor()
{
	destroy_sample(filt_sample);
	destroy_sample(orig_sample);
}

/**
 * @brief dispatch an event, external interface
 * 
 * This store the given event in the global event variable. 
 * When player thread will see event is not zero, it will 
 * really execute the dispatch behavior.
 * 
 * @param evt event to dispatch to the player
 */
void player_dispatch(player_event_t evt)
{
	pthread_mutex_lock(&player_event_mutex);
	player_event = evt;
	pthread_mutex_unlock(&player_event_mutex);
}

/**
 * @brief start the player thread
 * 
 * @param task_par thread parameter with which start the player thread
 * @return pthread_t* pointer to the player thread identificator
 */
pthread_t *player_start(task_par_t *task_par)
{
	struct sched_param mypar;
	pthread_attr_t attr;

	if (task_par != NULL)
	{
		tp = *task_par;
	}

	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	mypar.sched_priority = tp.priority;
	pthread_attr_setschedparam(&attr, &mypar);
	pthread_create(&tid, &attr, player_run, &tp);

	return &tid;
}

/**
 * @brief player thread routine
 * 
 * First execute some update operations, and then check if for global event is
 * different from empty event. If not it calls the dipatch.
 * 
 * @param[in] arg  argument passed to the routine(actually nothing is passed)
 * @return void* pointer to the variable returned by the thread(actually nothing)
 */
static void *player_run(void *arg)
{
	player_event_t evt;
	set_period(&tp);

	while (1)
	{
		if (p.state != STOP && p.state != PAUSE)
		{
			// allegro set position = -1 when the song reached the end.
			if (voice_get_position(v) < 0)
			{
				player_stop();
			}
			else
			{
				pos = voice_get_position(v);
				p.time = (((float)pos) / ((float)filt_sample->freq));
				// Online Filtering
				player_filt();
				// Spectogram update when reproducing
				sample_to_float(filt_sample, &p.time_data, pos, 1);
				update_spectogram(orig_sample, p.orig_spect);
				update_spectogram(filt_sample, p.filt_spect);
			}
		}

		pthread_mutex_lock(&player_event_mutex);
		evt = player_event;
		player_event.sig = 0;
		pthread_mutex_unlock(&player_event_mutex);
		// event different from empty
		if (evt.sig != 0)
			player_dispatch_body(evt);

		if (_player_exit)
		{
			player_xtor();
			pthread_exit(NULL);
		}
		if (deadline_miss(&tp))
			printf("PLAYER MISS\n");
		wait_for_period(&tp);
	}
	return NULL;
}

/**
 * @brief notice the player thread to exit
 */
void player_exit()
{
	_player_exit = 1;
}