/**
 * @file player_test.c
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief unit test player
 * @version 0.1
 * @date 2019-03-23
 *
 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include <unistd.h>

#include "player/player.h"
#include <allegro.h>

#include <criterion/criterion.h>

#define TEST_AUDIO_FILES_DIR "/home/universita/workspace/github.com/stiflerGit/AudioPlayer/TestAudioFiles/"
#define TEST_FILE "test.wav"

const player_event_t reset_event = {STOP_SIG, 0.0};

static void init()
{
	allegro_init();
	install_sound(DIGI_AUTODETECT, 0, 0);
}

static void fini()
{
	allegro_exit();
}

// TestSuite(unit, .init = init, .fini = fini);

// Test(unit, player_dispatch)
// {
// 	typedef struct
// 	{
// 		char *name;
// 		player_event_t args;
// 		wants;
// 	} testcase;
// 	testcase testcases[] = {
// 		{.name = "gain on filt"},
// 	};

// 	testcase *t = testcases;
// 	for (int i = 0; t->name != NULL;)
// 	{
// 		t = &testcases[i];
// 		player_dispatch(t->args);
// 	}
// }

TestSuite(transitions, .init = init, .fini = fini);

Test(transitions, reproducing_state)
{
	struct testcase
	{
		char *name;
		int wait_time; /**< wait_time between an input and its next */
		player_event_t *input_sequence;
		int input_sequence_size;
		player_state_t expected_state;
		float expected_elapsed_time;
	};
	struct testcase testcases[5] = {
		{.name = "test stop",
		 .wait_time = 1,
		 .input_sequence = (player_event_t[2]){{PLAY_SIG, 0},
											   {STOP_SIG, 0}},
		 .input_sequence_size = 2,
		 .expected_state = STOP,
		 .expected_elapsed_time = 1},
		{.name = "test play",
		 .wait_time = 1,
		 .input_sequence = (player_event_t[2]){{PLAY_SIG, 0},
											   {PLAY_SIG, 0}},
		 .input_sequence_size = 2,
		 .expected_state = PLAY,
		 .expected_elapsed_time = 1},
		{.name = "test pause",
		 .wait_time = 1,
		 .input_sequence = (player_event_t[2]){{PLAY_SIG, 0},
											   {PAUSE_SIG, 0}},
		 .input_sequence_size = 2,
		 .expected_state = PAUSE,
		 .expected_elapsed_time = 1},
		{.name = "test rewind",
		 .wait_time = 1,
		 .input_sequence = (player_event_t[2]){{PLAY_SIG, 0},
											   {RWND_SIG, 0}},
		 .input_sequence_size = 2,
		 .expected_state = STOP,
		 .expected_elapsed_time = 1},
		{.name = "test forward",
		 .wait_time = 1,
		 .input_sequence = (player_event_t[2]){{PLAY_SIG, 0},
											   {FRWD_SIG, 0}},
		 .input_sequence_size = 2,
		 .expected_state = FORWARD,
		 .expected_elapsed_time = 1}};

	player_init(TEST_AUDIO_FILES_DIR TEST_FILE);
	player_start(NULL);

	struct testcase *t;
	for (int i = 0; i < 5; i++)
	{
		t = &testcases[i];
		// first bring the machine in the correct state
		for (int j = 0; j < t->input_sequence_size - 1; j++)
		{
			float start_time = player_get_time();
			player_dispatch(t->input_sequence[j]);
			// sleep wait time
			sleep(t->wait_time);
		}
		player_dispatch(t->input_sequence[t->input_sequence_size - 1]);
		// sleep wait time
		sleep(t->wait_time);
		// check last state an output
		{
			player_state_t player_state = player_get_state();
			cr_expect_eq(player_state, t->expected_state,
						 "%s", t->name);
		}
		player_dispatch(reset_event);
		sleep(1);
	}

	player_exit();
}

Test(transitions, stop_state)
{
	struct testcase
	{
		char *name;
		int wait_time; /**< wait_time of the test in seconds */
		player_event_t *input_sequence;
		int input_sequence_size;
		player_state_t expected_state;
		float expected_elapsed_time;
	};
	struct testcase testcases[5] = {
		{.name = "play transition",
		 .wait_time = 2,
		 .input_sequence = (player_event_t[1]){{PLAY_SIG, 0}},
		 .input_sequence_size = 1,
		 .expected_state = PLAY,
		 .expected_elapsed_time = 2.0},
		{.name = "forward transition",
		 .wait_time = 2,
		 .input_sequence = (player_event_t[1]){{FRWD_SIG, 0}},
		 .input_sequence_size = 1,
		 .expected_state = FORWARD,
		 .expected_elapsed_time = 2.5},
		{.name = "stop transition",
		 .wait_time = 2,
		 .input_sequence = (player_event_t[1]){{STOP_SIG, 0}},
		 .input_sequence_size = 1,
		 .expected_state = STOP,
		 .expected_elapsed_time = 0.0},
		{.name = "pause transition",
		 .wait_time = 2,
		 .input_sequence = (player_event_t[1]){{PAUSE_SIG, 0}},
		 .input_sequence_size = 1,
		 .expected_state = STOP,
		 .expected_elapsed_time = 0.0},
		{.name = "rewind transition",
		 .wait_time = 2,
		 .input_sequence = (player_event_t[1]){{RWND_SIG, 0}},
		 .input_sequence_size = 1,
		 .expected_state = STOP,
		 .expected_elapsed_time = 0.0},
	};

	player_init(TEST_AUDIO_FILES_DIR TEST_FILE);
	player_start(NULL);

	struct testcase *t;
	for (int i = 0; i < 5; i++)
	{
		t = &testcases[i];
		for (int j = 0; j < t->input_sequence_size; j++)
		{
			float start_time = player_get_time();
			player_dispatch(t->input_sequence[j]);
			sleep(t->wait_time);
			player_state_t player_state = player_get_state();
			cr_expect_eq(player_state, t->expected_state,
						 "%s", t->name);
		}
		player_dispatch(reset_event);
		sleep(1);
	}

	player_exit();
}
