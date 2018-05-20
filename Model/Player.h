#ifndef PLAYER_H_
#define PLAYER_H_

#define PLAYER_MAX_FREQ		(44100)			/**< Max sample per seconds. */
#define PLAYER_MAX_SMPL_SIZE	(2)			/**< Max no. Byte per sample. */
#define PLAYER_MAX_NCH		(1)			/**< Max no. Channels. */
#define PLAYER_WINDOW_SIZE	(8192)
#define PLAYER_WINDOW_SIZE_CPX	((PLAYER_WINDOW_SIZE/2)+1)
#define PLAYER_NFILT		(4)			/**< No. Filters implementig EQ. */
typedef enum {
	EMPTY_SIG = 0,
	TICK_SIG,
	STOP_SIG,	/**< Stop the track. */
	PLAY_SIG,	/**< Play the track. */
	PAUSE_SIG,	/**< Pause the track, */
	RWND_SIG,	/**< Fast rewind the track. */
	FRWD_SIG,	/**< Fast forward the track. */
	VOL_SIG,	/**< Volume changes.
			 *	@param	value	volume value;
			 */
	JUMP_SIG,	/**< Jump to a specific play time. */
	FILTLOW_SIG,	/**< Filter low frequencies (20Hz - 500Hz). */
	FILTMED_SIG,	/**< Filter medium frequencies (500Hz - 2000Hz). */
	FILTMEDHIG_SIG,	/**< Filter medium-high frequencies (2000Hz - 8000Hz). */
	FILTHIG_SIG	/**< Filter high frequencies (8000Hz - 16000Hz). */
} psignal;

typedef struct {
	psignal	sig;
	float	val;	/**< Volume value, in case sig = VOL_SIG;
			 **< Frequencies gain value, in case sig = FILT*;
			 **< Time to start play, in case sig = GOTO_SIG;
			 */
} pevent;

typedef enum {
	STOP, PLAY, PAUSE, REWIND, FORWARD,
} pstate;

#define MAX_GAIN	12
#define MIN_GAIN	-12

typedef struct {
	float	gain;		/**< Decibel gain. */
	float	low_bnd;	/**< Frequency lower bound. */
	float	upp_bnd;	/**< Frequency upper bound. */
} filter;

typedef struct{
	pstate		state;					/**< The player State. */
	char 		trackname[100];				/**< Track Name. */
	float 		time;					/**< Actual reproducing time in sec. */
	float 		duration;				/**< Total track duration in sec. */
	float		spectogram[PLAYER_WINDOW_SIZE_CPX];	/**< Spectrogram of the reproducing window. */
	float		dynamic_range;				/**< Decibel range of each spect. term. */
	float		freq_spacing;				/**< Frequency spacing between each spect. term */
	unsigned int 	volume;					/**< Reproducing volume [0-100]. */
	filter		equaliz[PLAYER_NFILT];			/**< Set of filters implementing the band equalizator . */
}Player;

extern Player p;
extern pevent evt;

/*
 * @brief	Initialize the Player p
 *
 *@param[in]	path	path were the song is located
 *@return				-1 in case of error, 0 otherwise
 */
void pinit(const char *path);

/*
 * @brief	dispatch the user events to the player
 *
 * @param[in]	sig	specify the signal (i.e. the event)
 * 					that has to be dispatched
 */
void pdispatch();

/*
 * @brief	print on the stdout the player info
 */
void pprint();

void print_spect();

#endif /* PLAYER_H_ */
