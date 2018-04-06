/*
 * Player.h
 *
 *  Created on: May 7, 2017
 *      Author: stefano
 */
#ifndef PLAYER_H_
#define PLAYER_H_

#define PLAYER_MAX_FREQ			(44100)			/**< Max sample per seconds. */
#define PLAYER_MAX_SMPL_SIZE	(2)				/**< Max no. Byte per sample. */
#define PLAYER_MAX_NCH			(1)				/**< Max no. Channels. */
#define PLAYER_WINDOW_SIZE		(8192)
#define PLAYER_WINDOW_SIZE_CPX	((PLAYER_WINDOW_SIZE/2)+1)
#define PLAYER_MAX_NFIL			(4)				/**< Max no. Filters: */

typedef enum {
	EMPTY_SIG = 0,
	TICK_SIG,
	STOP_SIG,		/**< Stop the track. */
	PLAY_SIG,		/**< Play the track. */
	PAUSE_SIG,		/**< Pause the track, */
	RWND_SIG,		/**< Fast rewind the track. */
	FRWD_SIG,		/**< Fast forward the track. */
	VOL_SIG,		/**< Volume changes.
					 *	@param	value	volume value;
					 */
	EQLZ_SIG		/**< Equalize the track frequencies
	 	 	 	 	 *	@param	filt	value of the filters
	 	 	 	 	 */
} psignal;

typedef struct {
	psignal		sig;
	union data {
		int		val;					/**< Volume value, in case sig = VOL_SIG */
		float	filt[PLAYER_MAX_NFIL];	/**< Filters' gains, in case sig = EQLZ_SIG */
	} data;
} pevent;

typedef enum {
	STOP, PLAY, PAUSE, REWIND, FORWARD,
} pstate;

typedef struct{
	pstate			state;								/**< The player State. */
	char 			trackname[100];						/**< Track Name. */
	float 			time;								/**< Actual reproducing time in sec. */
	float 			duration;							/**< Total track duration in sec. */
	unsigned int 	volume;								/**< Reproducing volume [0-255]. */
	float			spectogram[PLAYER_WINDOW_SIZE_CPX];	/**< Spectrogram of the reproducing window. */
	float			dynamic_range;						/**< Decibel range of each spect. term. */
	float			freq_spacing;						/**< Frequency spacing between each spect. term */
}Player;

//extern Player p;

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
void pdispatch(pevent evt);

/*
 * @brief	print on the stdout the player info
 */
void pprint();

void print_spect();

#endif /* PLAYER_H_ */
