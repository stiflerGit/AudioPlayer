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
#include <cmocka.h>

#define TEST_AUDIO_FILES_DIR "/home/universita/workspace/github.com/stiflerGit/AudioPlayer/TestAudioFiles/"
#define TEST_FILE "test.wav"

const player_event_t reset_event = {STOP_SIG, 0};

void init()
{
  allegro_init();
  install_sound(DIGI_AUTODETECT, 0, 0);
}

static void test_player_reproducing_state(void **state)
{
  struct testcase
  {
    char *name;
    int wait_time; /**< wait_time between an input and its next */
    player_event_t *input_sequence;
    int input_sequence_size;
    player_state_t expected_state;
    float expected_elapsed_time;
    /* data */
  };
  struct testcase testcases[] = {
      {name : "test stop",
       wait_time : 1.0,
       input_sequence : (player_event_t[2]){{PLAY_SIG, 0}, {STOP_SIG, 0}},
       input_sequence_size : 2,
       expected_state : STOP,
       expected_elapsed_time : 1},
      {
        name : "test play",
        wait_time : 1.0,
        input_sequence : (player_event_t[2]){{PLAY_SIG, 0}, {PLAY_SIG, 0}},
        input_sequence_size : 2,
        expected_state : PLAY,
        expected_elapsed_time : 1
      },
      {
        name : "test pause",
        wait_time : 0.0,
        input_sequence : (player_event_t[2]){{PLAY_SIG, 0}, {PAUSE_SIG, 0}},
        input_sequence_size : 2,
        expected_state : PAUSE,
        expected_elapsed_time : 1
      },
      {
        name : "test rewind",
        wait_time : 0.0,
        input_sequence : (player_event_t[2]){{PLAY_SIG, 0}, {RWND_SIG, 0}},
        input_sequence_size : 2,
        expected_state : REWIND,
        expected_elapsed_time : 1
      },
      {
        name : "test forward",
        wait_time : 0.0,
        input_sequence : (player_event_t[2]){{PLAY_SIG, 0}, {FRWD_SIG, 0}},
        input_sequence_size : 2,
        expected_state : FORWARD,
        expected_elapsed_time : 1
      }};
}

static void test_player_stop_state(void **state)
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
      {name : "stop play transition",
       wait_time : 2,
       input_sequence : (player_event_t[1]){{PLAY_SIG, 0}},
       input_sequence_size : 1,
       expected_state : PLAY,
       expected_elapsed_time : 2.0},
      {name : "stop forward transition",
       wait_time : 2,
       input_sequence : (player_event_t[1]){{FRWD_SIG, 0}},
       input_sequence_size : 1,
       expected_state : FORWARD,
       expected_elapsed_time : 2.5},
      {name : "stop stop transitions",
       wait_time : 2,
       input_sequence : (player_event_t[1]){{STOP_SIG, 0}},
       input_sequence_size : 1,
       expected_state : STOP,
       expected_elapsed_time : 0.0},
      {name : "stop pause transitions",
       wait_time : 2,
       input_sequence : (player_event_t[1]){{PAUSE_SIG, 0}},
       input_sequence_size : 1,
       expected_state : STOP,
       expected_elapsed_time : 0.0},
      {name : "stop rewind transitions",
       wait_time : 2,
       input_sequence : (player_event_t[1]){{RWND_SIG, 0}},
       input_sequence_size : 1,
       expected_state : STOP,
       expected_elapsed_time : 0.0},
  };

  player_init(TEST_AUDIO_FILES_DIR TEST_FILE);
  player_start(NULL);

  struct testcase *t;
  for (int i = 0; i < NTEST; i++)
  {
    t = &testcases[i];
    printf("test: %s\n", t->name);
    for (int j = 0; j < t->input_sequence_size; j++)
    {
      float start_time = p.time;
      player_dispatch(t->input_sequence[j]);
      // sleep one second
      sleep(t->wait_time);
      assert_true(p.state == t->expected_state);
      // assert_true(p.time - start_time >= t->expected_elapsed_time);
    }
    player_dispatch(reset_event);
    sleep(1);
  }

  player_exit();
}

int main(void)
{
  init();
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_player_stop_state),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}

//gcc -o player_test player_test.c player.c equalizer.c ../ptask.c -I../../include -lpthread -lm -lfftw3f -lcmocka  `allegro-config --libs`
