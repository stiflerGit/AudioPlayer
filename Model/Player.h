/**
 * @file	Player.h
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Audio Player, events and state definitions.
 *
 * This contains all data types needed to interact with the Audio Player.
 * It defines only two function the pinit to initialize the player, and
 * the pdispatch, to dispatch events to the player.
 * The rest of the file is composed by the definition of data structures,
 * in particularly two class of structures:
 * 1 - Input to the player (Signal, Events)
 * 2 - Output of the player (State, Filters)
 *
 * @bug Deadline Miss for the filters events.
 */


#ifndef PLAYER_H_
#define PLAYER_H_

#define PLAYER_MAX_FREQ		(44100)			/**< Max sample per seconds. */
#define PLAYER_MAX_SMPL_SIZE	(2)			/**< Max no. Byte per sample. */
#define PLAYER_MAX_NCH		(1)			/**< Max no. Channels. */
#define PLAYER_WINDOW_SIZE	(8192)
#define PLAYER_WINDOW_SIZE_CPX	((PLAYER_WINDOW_SIZE/2)+1)
#define PLAYER_NFILT		(4)			/**< No. Filters implementig EQ. */

/**
 * @brief	Signals by means of interact with the Player.
 *
 * Player accept this signals. Create an event with this signal
 * make possible to change player outputs and listen for different
 * effects. All signals different from this are ignored.
 *
 */
typedef enum {
	EMPTY_SIG = 0,	/**< For debug scope only. Do nothing. */
	TICK_SIG,	/**< Tick signal. */
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

/**
 * @brief	Inputs for the player dispatch.
 *
 * Not all signals carries an ON-OFF information. Events wrap signal
 * and a float value for that kind of signal that need an additional
 * information.
 */
typedef struct {
	psignal	sig;	/**< Event signal. */
	float	val;	/**< Volume value, in case sig = VOL_SIG;
			 **< Frequencies gain value, in case sig = FILT*_SIG;
			 **< Time to start play, in case sig = JUMP_SIG;
			 */
} pevent;

/**
 * @brief	Player states.
 *
 * In each moment the player could be in a different state depending
 * on inputs events it received. This are all Player states.
 */
typedef enum {
	STOP,		/**< No audio reproducing. Position: start od the song */
	PLAY,		/**< Reproducing normal speed. */
	PAUSE,		/**< No audio reproducing. Position: last before pause state. */
	REWIND,		/**< Reproducing backward faster than normal speed. */
	FORWARD,	/**< Reproducing faster. */
} pstate;

#define MAX_GAIN	12	/**< Maximum Decibel gain of filters. */
#define MIN_GAIN	-12	/**< Minimum Decibel gain of filters. */

/**
 * @brief	Filter definition for cut/gain an audio frequency.
 */
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

/**
 * @brief	Initialize the Player p
 *
 * @param[in]	path	path were the song is located
 * @return		-1 in case of error, 0 otherwise
 */
void pinit(const char *path);

/**
 * @brief	dispatch the user events to the player
 *
 * @param[in]	sig	specify the signal (i.e. the event)
 * 			that has to be dispatched
 */
void pdispatch();

/**
 * @brief	print on the stdout the player info
 */
void pprint();

void print_spect();

#endif /* PLAYER_H_ */
