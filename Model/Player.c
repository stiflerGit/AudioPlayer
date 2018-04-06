/*
 * Player.c
 *
 *  Created on: May 7, 2017
 *      Author: stefano
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include <allegro.h>
#include "Player.h"

#define handle_error(s)	\
	do{ perror(s); exit(EXIT_FAILURE);} while(1)

#define modulus(cpx)	(sqrt(((cpx)[0] * (cpx)[0]) + ((cpx)[1] * (cpx)[1])))
#define phase(cpx)		(atan2f((cpx)[1], (cpx)[0]))

Player		p;	/**< The player struct. */

static pstate		mystate;	/**< State of the player. */
static int		pos;		/**< Reproducing position. */
static int 		v;		/**< Allegro voice associated to player. */
static SAMPLE 		*orig_sample;	/**< Original Sample. */
static SAMPLE 		*filt_sample;	/**< Filtered Sample. */

struct filter{
	float	gain;
	float	lower_bound;
	float	upper_bound;
};

/*
 * Set of filters that
 * implements the Band Equalizer
 */
static struct filter filters[] = {
				{ 1, 20, 500 },		/**< Low Frequencies. */
				{ 1, 500, 2000 },	/**< Medium Frequencies. */
				{ 1, 2000, 8000 },	/**< Medium-High Frequencies. */
				{ 1, 8000, 16000 }	/**< High Frequencies. */
};

/******************************************************************************
 * Player Events
 *****************************************************************************/
static void PlayerPlay();
static void PlayerPause();
static void PlayerStop();
static void PlayerRewind();
static void PlayerForward();
//*****************************************************************************

/******************************************************************************
 * Player Methods
 *****************************************************************************/
void pprint(){
	printf("trackname: %s\nvolume: %d\ntime %f\nduration %f\n",
		p.trackname, p.volume, p.time, p.duration);
	printf("dynamic range: %f\nfrequency spacing:%f\n", p.dynamic_range, p.freq_spacing);
}
//*****************************************************************************

static void get_trackname(char *name, const char *path)
{
char	*name_, *tok;
char	path_[256];

	if (path != NULL) {
		strcpy(path_, path);
		name_ = path_;
		tok = strtok(path_, "/");
		while (tok != NULL) {
			name_ = tok;
			tok = strtok(NULL, "/");
		}
		strcpy(name, name_);
	}
}

static void algr_load_smpl(const char * path, SAMPLE **s)
{
char	pass;
char	err[1024];

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
	if (pass) {
		printf("bits: %d\nstereo: %d\nfreq: %d\nlen: %ld\n", (*s)->bits,
				(*s)->stereo, (*s)->freq, (*s)->len);
	} else {
		destroy_sample(*s);
		printf("%s\n", err);
		exit(EXIT_FAILURE);
	}
}

static int sample_to_float(const SAMPLE *s, float *f, int i, int N)
{
int		j;		/**< sample data index */
int16_t	d16;	/**< buff to get the original signed value. */
int8_t	d8;

	for (j = 0; j < N && (i + j) < s->len; j++) {
		if (s->bits == 16) {
			d16 = le16toh(((int16_t *)(s->data))[i+j]) ^ 0x8000;
			f[j] = (float) d16;
		} else if (s->bits == 8) {
			d8 = ((uint8_t *) (s->data))[i+j] ^ 0x80;
			f[j] = (float) d8;
		}
	}
	return j;
}

static int float_to_sample(const float *f, SAMPLE *s, int i, int N)
{
int			j;		/**< sample data index */
uint16_t	d16;	/**< buff to get the original signed value. */

	for (j = 0; j < N && (i + j) < s->len; j++) {
		if (s->bits == 16) {
			d16 = ((int16_t) round(f[j])) ^ 0x8000;
			((uint16_t *) s->data)[i+j] = htole16(d16);
		} else if (s->bits == 8) {
			((uint8_t *) s->data)[i+j] = ((uint8_t) round(f[j])) ^ 0x80;
		}
	}
	return j;
}

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

static void FFT(const float *in, fftwf_complex *out)
{
fftwf_plan	dft_p;				/**< Direct f.t. configuration structure. */
float		inbuff[PLAYER_WINDOW_SIZE];	/**< Need a buffer, cause create plan destroy input data, */

	dft_p = fftwf_plan_dft_r2c_1d(PLAYER_WINDOW_SIZE, inbuff, out,
			FFTW_ESTIMATE);
	memcpy(inbuff, in, sizeof(inbuff));
	fftwf_execute(dft_p);
	fftwf_destroy_plan(dft_p);
}

static void IFFT(const fftwf_complex *in, float *out)
{
int			i;
fftwf_plan		ift_p;			/**< Inverse f.t. configuration structure. */
fftwf_complex	inbuff[PLAYER_WINDOW_SIZE_CPX];	/**< Need a buffer, cause create plan destroy input data, */

	ift_p = fftwf_plan_dft_c2r_1d(PLAYER_WINDOW_SIZE, inbuff, out,
			FFTW_ESTIMATE);
	memcpy(inbuff, in, sizeof(inbuff));
	fftwf_execute(ift_p);
	for (i = 0; i < PLAYER_WINDOW_SIZE; i++) {
		out[i] /= PLAYER_WINDOW_SIZE;
	}
	fftwf_destroy_plan(ift_p);
}

void print_sample(SAMPLE *s)
{
unsigned char	b[2];

	for (int i = 0; i < s->len; i++) {
		printf("===== sample %d =====\n", i);
		if (s->bits == 16) {
			b[1] = (unsigned char) ((uint16_t *) s->data)[i];
			b[0] = (unsigned char) (((uint16_t *) s->data)[i] >> 8);
			printf("%d\t%d\n", b[1], b[0]);
		} else {
			b[0] = ((char *) s->data)[i];
			printf("%d\n", b[0]);
		}
	}
}

void print_fbuff(const float *buff, unsigned int N)
{
int	i;

	printf("const float buff[%d] = {", N);
	for (i = 0; i < N - 1; i++) {
		if (i % 5 == 0)
			printf("\n");
		printf("%d,\t", (int) buff[i]);
	}
	printf("%d\n}\n", (int) buff[N - 1]);
}

static void update_spectogram()
{
int				i;
int				ret;
float			timedata[PLAYER_WINDOW_SIZE];
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
static int		max = 0;

	i = (pos - PLAYER_WINDOW_SIZE / 2 < 0) ? 0 : pos - PLAYER_WINDOW_SIZE / 2;
	ret = sample_to_float((const SAMPLE *) orig_sample, timedata, i,
			PLAYER_WINDOW_SIZE);
	// zero pad in case there aren't enough time data
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
	// apply blackman harris window f. to better isolate frequency
	for(i = 0; i < PLAYER_WINDOW_SIZE; i++){
		timedata[i] *= blackman_harris(i);
	}

	FFT(timedata, freqdata);
	// compute magnitude and find the maximum value
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		p.spectogram[i] = modulus(freqdata[i]);
		if(p.spectogram[i] > max)
			max = p.spectogram[i];
	}
	// use maximum value to normalize power spectrum
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		p.spectogram[i] /= max;
		p.spectogram[i] = 20.0f * log10f( p.spectogram[i]);
		p.spectogram[i] = (p.spectogram[i] + p.dynamic_range) / p.dynamic_range;
		if(p.spectogram[i] < 0)	p.spectogram[i] = 0;
		p.spectogram[i] = (int) (p.spectogram[i] * 100);
	}
}

void print_spect()
{
	print_fbuff(p.spectogram, PLAYER_WINDOW_SIZE_CPX);
}

static void filt(fftwf_complex freqdata[])
{
int		j, k;
float	mod, ph;

	for (j = 0; j < PLAYER_WINDOW_SIZE_CPX; j++) {
		for (k = 0; k < PLAYER_MAX_NFIL; k++) {
			if ((j * p.freq_spacing) > filters[k].lower_bound
					&& (j * p.freq_spacing) < filters[k].upper_bound) {
				mod = modulus(freqdata[j]) * filters[k].gain;
				ph = phase(freqdata[j]);
				freqdata[j][0] = mod * cosf(ph);
				freqdata[j][1] = mod * sinf(ph);
			}
		}
	}
}

static void PlayerFilt()
{
int				i;
int				ret;
float			timedata[PLAYER_WINDOW_SIZE];
fftwf_complex	freqdata[PLAYER_WINDOW_SIZE_CPX];
SAMPLE			*new_filt_sample;
char			start;

	new_filt_sample = create_sample(orig_sample->bits, orig_sample->stereo,
			orig_sample->freq, orig_sample->len);

	/*
	 * first window is bigger than all the others
	 */
	ret = sample_to_float((const SAMPLE *) orig_sample, timedata, 0,
			PLAYER_WINDOW_SIZE);
	if (ret < PLAYER_WINDOW_SIZE)
		memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
	FFT(timedata, freqdata);
	filt(freqdata);
	IFFT(freqdata, timedata);
	float_to_sample((const float *) timedata, new_filt_sample, 0,
			3 * (PLAYER_WINDOW_SIZE / 4));


	for (i = 1; i < 2 * (filt_sample->len / PLAYER_WINDOW_SIZE); i++) {
		ret = sample_to_float((const SAMPLE *) orig_sample, timedata,
				i * (PLAYER_WINDOW_SIZE/2), PLAYER_WINDOW_SIZE);
		if(ret < PLAYER_WINDOW_SIZE)
			memset(&timedata[ret], 0, PLAYER_WINDOW_SIZE - ret);
		FFT(timedata, freqdata);
		filt(freqdata);
		IFFT(freqdata, timedata);
		float_to_sample((const float *) &timedata[PLAYER_WINDOW_SIZE/4], new_filt_sample,
				(i * (PLAYER_WINDOW_SIZE / 2)) + PLAYER_WINDOW_SIZE / 4,
				PLAYER_WINDOW_SIZE / 2);
	}

	start = 0;
	if (voice_get_position(v) >= 0)
		start = 1;
	reallocate_voice(v, new_filt_sample);
	destroy_sample(filt_sample);
	filt_sample = new_filt_sample;
	voice_set_position(v, pos);
	if (start)
		voice_start(v);
}


void pinit(const char *path)
{
	algr_load_smpl(path, &orig_sample);
	algr_load_smpl(path, &filt_sample);

	p.state = mystate = STOP;
	p.time = pos = 0;
	get_trackname(p.trackname, path);
	p.duration = ((float) (orig_sample->len / orig_sample->freq));
	p.volume = 255;
	memset(p.spectogram, 0, sizeof(p.spectogram));
	p.dynamic_range = fabsf(20.0f * log10f(1.0f / (1 << orig_sample->bits)));
	p.freq_spacing = ((float) orig_sample->freq) / PLAYER_WINDOW_SIZE;

	// allocating the sample
	v = allocate_voice(filt_sample);
	if (v < 0)
		handle_error("allocate_voice");
	voice_set_playmode(v, PLAYMODE_PLAY);
}

void pdispatch(pevent evt)
{
int	i;

	if (voice_get_position(v) < 0 && mystate != STOP && mystate != PAUSE) {
		PlayerStop();
	}
	else{
		pos = voice_get_position(v);
		p.time = pos / orig_sample->freq;
		update_spectogram();
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
		voice_set_volume(v, evt.data.val);
		break;
	case EQLZ_SIG:
		for (i = 0; i < PLAYER_MAX_NFIL; i++){
			filters[i].gain = evt.data.filt[i];
		}
		PlayerFilt();
		break;
	default:
		break;
	}
}

static void PlayerStop()
{
	if(mystate != PAUSE)
		voice_stop(v);
	if (mystate == REWIND || mystate == FORWARD) {
		if (mystate == REWIND)
			voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_frequency(v, orig_sample->freq);
	}
	voice_set_position(v, 0);
	memset(p.spectogram, 0, sizeof(p.spectogram));
	p.time = pos = 0;
	p.state = mystate = STOP;
}

static void PlayerPlay()
{
	if (mystate == STOP || mystate == PAUSE)
		voice_start(v);
	if (mystate == REWIND || mystate == FORWARD) {
		if (mystate == REWIND){
			voice_set_playmode(v, PLAYMODE_FORWARD);
			voice_set_position(v, pos);
		}
		voice_set_frequency(v, orig_sample->freq);
	}
	p.state = mystate = PLAY;
}

static void PlayerPause()
{
	if(mystate != STOP && mystate != PAUSE){
		voice_stop(v);
	}
	if (mystate == REWIND || mystate == FORWARD){
		voice_set_frequency(v, orig_sample->freq);
		if(mystate == REWIND){
			voice_set_playmode(v, PLAYMODE_FORWARD);
			voice_set_position(v, pos);
		}
	}
	p.state = mystate = PAUSE;
}

static void PlayerRewind()
{
	if(mystate != REWIND && mystate != STOP){
		voice_set_playmode(v, PLAYMODE_BACKWARD);
		voice_set_position(v, pos);
		voice_set_frequency(v, (((float) orig_sample->freq) * 1.25));
	}
	else if (mystate == REWIND)
		voice_set_frequency(v, 2 * voice_get_frequency(v));
	if (mystate == PAUSE)
		voice_start(v);
	p.state = mystate = REWIND;
}

static void PlayerForward()
{
	if (mystate != FORWARD){
		voice_set_frequency(v, (((float) orig_sample->freq) * 1.25));
	}
	else{
		voice_set_frequency(v, 2 * voice_get_frequency(v));
	}
	if (mystate == STOP || mystate == PAUSE){
		voice_start(v);
	}
	if (mystate == REWIND){
		voice_set_playmode(v, PLAYMODE_FORWARD);
		voice_set_position(v, pos);
	}
	p.state = mystate = FORWARD;
}
