/**
 * @file	player.h
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Audio Player_t, events and state definitions.
 *
 * This contains all data types and functions needed to interact with 
 * the Audio Player_t.
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <pthread.h>

#include "ptask.h"

#define PLAYER_MAX_FREQ (44100)   /**< Max sample per seconds. */
#define PLAYER_MAX_SMPL_SIZE (2)  /**< Max no. Byte per sample. */
#define PLAYER_MAX_NCH (1)		  /**< Max no. Channels. */
#define PLAYER_WINDOW_SIZE (8192) /**< Size of the Windows for spectogram \
				  computation. */
#define PLAYER_WINDOW_SIZE_CPX ((PLAYER_WINDOW_SIZE / 2) + 1)
#define PLAYER_NFILT (4) /**< No. Filters implementig EQ. */

#ifndef MAX_GAIN
#define MAX_GAIN 15 /**< max gain in deciBel. */
#endif

#define PLAYER_PERIOD 80
#define PLAYER_DEADLINE 80
#define PLAYER_PRIORITY 20
/**
 * @brief	Signals by means of interact with the Player_t.
 *
 * Player_t accept this signals. Create an event with this signal
 * make possible to change player outputs and listen for different
 * effects. All signals different from this are ignored.
 *
 */
typedef enum
{
	EMPTY_SIG,		/**< Do nothing. */
	TICK_SIG,		/**< Tick signal. */
	STOP_SIG,		/**< Stop the track. */
	PLAY_SIG,		/**< Play the track. */
	PAUSE_SIG,		/**< Pause the track, */
	RWND_SIG,		/**< Fast rewind the track. */
	FRWD_SIG,		/**< Fast forward the track. */
	VOL_SIG,		/**< Volume changes.
			 		*	@param	value	volume value;
			 		*/
	JUMP_SIG,		/**< Jump to a specific play time. */
	FILTLOW_SIG,	/**< Filter low frequencies (20Hz - 500Hz). */
	FILTMED_SIG,	/**< Filter medium frequencies (500Hz - 2000Hz). */
	FILTMEDHIG_SIG, /**< Filter medium-high frequencies (2000Hz - 8000Hz).*/
	FILTHIG_SIG		/**< Filter high frequencies (8000Hz - 16000Hz). */
} player_signal_t;

/**
 * @brief	Inputs for the player dispatch.
 *
 * Not all signals carries an ON-OFF information. Events wrap signal
 * and a float value for that kind of signal that need an additional
 * information.
 */
typedef struct
{
	player_signal_t sig; /**< Event signal. */
	float val;			 /**< Volume value, in case sig = VOL_SIG;
			 **< Frequencies gain value, in case sig = FILT*_SIG;
			 **< Time to start play, in case sig = JUMP_SIG;
			 */
} player_event_t;

/**
 * @brief	Player_t states.
 *
 * In each moment the player could be in a different state depending
 * on inputs events it received. This are all Player_t states.
 */
typedef enum
{
	STOP,	/**< No audio reproducing. Position: start od the song. */
	PLAY,	/**< Reproducing normal speed. */
	PAUSE,   /**< No audio reproducing. Position: last before pause state. */
	REWIND,  /**< Reproducing backward faster than normal speed. */
	FORWARD, /**< Reproducing faster. */
} player_state_t;

typedef struct
{
	player_state_t state; /**< The player State. */
	char trackname[100];  /**< Track Name. */
	float time;			  /**< Actual reproducing time in sec. */
	float duration;		  /**< Total track duration in sec. */
	float time_data;	  /**< Timedata. */
	int bits;			  /**< Bit depth of samples. */
	float orig_spect[PLAYER_WINDOW_SIZE_CPX];
	/**< Spectrogram of the original window. (not filtered song) */
	float filt_spect[PLAYER_WINDOW_SIZE_CPX];
	/**< Spectrogram of the reproducing window. (i.e. the filtered song) */
	float dynamic_range;		 /**< Decibel range of each spect. term.*/
	float freq_spacing;			 /**< Frequency spacing between each spect. term */
	unsigned int volume;		 /**< Reproducing volume [0-100]. */
	float eq_gain[PLAYER_NFILT]; /**< gain at each frequency. */
} Player_t;

/**
 * @brief Initialize the player
 * 
 * @param path audio file path
 */
void player_init(const char *path) __attribute__((nonnull(1)));

/**
 * @brief start the player thread
 * 
 * @param task_par    pointer to a struct definig task parameters(not mandatory)
 * @return pthread_t* pointer to the thread identificator
 */
pthread_t *player_start(task_par_t *task_par);

/**
 * @brief dispatch an event to the player
 * 
 * @param evt event to dispatch
 */
void player_dispatch(player_event_t evt);

/**
 * @brief shortcut for player_dispatch EXIT_SIG
 */
void player_exit();

/**
 * @brief 
 * 
 * @return player_state_t 
 */
player_state_t player_get_state();

/**
 * @brief 
 * 
 * @param dst 
 */
void player_get_trackname(char *dst);

/**
 * @brief 
 * 
 * @return float 
 */
float player_get_time();

/**
 * @brief 
 * 
 * @return float 
 */
float player_get_duration();

/**
 * @brief 
 * 
 * @return float 
 */
float player_get_time_data();

/**
 * @brief 
 * 
 * @return int 
 */
int player_get_bits();

/**
 * @brief 
 * 
 * @param dst 
 */
void player_get_orig_spect(float *dst);

/**
 * @brief 
 * 
 * @param dst 
 */
void player_get_filt_spect(float *dst);

/**
 * @brief 
 * 
 * @return float 
 */
float player_get_dynamic_range();

/**
 * @brief 
 * 
 * @return float 
 */
float player_get_freq_spacing();

/**
 * @brief 
 * 
 * @return unsigned int 
 */
unsigned int player_get_volume();

/**
 * @brief 
 * 
 * @param dst 
 */
void player_get_eq_gain(float *dst);

/**
 * @brief 
 * 
 * @param dst 
 */
void player_get_player(Player_t *dst);

#endif /* PLAYER_H_ */
