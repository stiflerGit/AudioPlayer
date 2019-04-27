/**
 * @file equalizer_test.c
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief unit test equalizer
 * @version 0.1
 * @date 2019-03-23
 *
 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include <criterion/criterion.h>

#include "defines.h"
#include "equalizer.c"

#define TEST_RESULTS_DIR "/tmp/github.com/stiflerGit/AudioPlayer/test/player/"

void init()
{
	struct stat st = {0};
	int ret;
	if (stat(TEST_RESULTS_DIR, &st) == -1)
	{
		ret = mkdir(TEST_RESULTS_DIR, 0700);
		if (ret < 0)
		{
			perror("making test dir");
			exit(EXIT_FAILURE);
		}
	}
}

// TODO: Give a correct name to the suite
TestSuite(idk, .init = init);

typedef struct
{
	int signal_freq;
	int sampling_freq;
	int size;
	float *data;
} signal_t;

/**
 * @brief
 *
 * @param signal_freq in hertz
 * @param sampling_freq in hertz
 * @param duration in milliseconds
 * @return signal_t* pointer to the new signal
 */
signal_t *signal_new(float amplitude, float signal_freq, float sampling_freq, float duration)
{
	float time_period;
	float y;
	signal_t *signal;
	signal = malloc(sizeof(signal_t));
	(*signal).signal_freq = signal_freq;
	(*signal).sampling_freq = sampling_freq;
	(*signal).size = (duration / 1000.0f * ((float)sampling_freq));
	(*signal).data = malloc((*signal).size * sizeof(float));

	time_period = 1.0f / sampling_freq;
	for (int i = 0; i < (*signal).size; i++)
	{
		y = sinf(2.0f * M_PI * signal_freq * i * time_period);
		(*signal).data[i] = amplitude * y;
	}
	return signal;
}

void signal_delete(signal_t *signal)
{
	free((*signal).data);
	free(signal);
	signal = NULL;
}

#define SIGNAL_PRINT(s)  \
	do                   \
	{                    \
		printf(#s);      \
		signal_print(s); \
	} while (0)

void signal_print(const signal_t *signal)
{
	printf("{ signal_freq: %d, ", (*signal).signal_freq);
	printf("sampling_freq: %d, ", (*signal).sampling_freq);
	printf("size: %d, ", (*signal).size);
	printf("data: [");
	for (int i = 0; i < (*signal).size; i++)
	{
		if (i % 10 == 0)
			printf("\n");
		if (i < (*signal).size - 1)
			printf("%f, ", (*signal).data[i]);
		else
			printf("%f ]", (*signal).data[i]);
	}
}

signal_t *signal_clone(const signal_t *src)
{
	signal_t *dst;

	dst = malloc(sizeof(signal_t));
	(*dst).signal_freq = (*src).signal_freq;
	(*dst).sampling_freq = (*src).sampling_freq;
	(*dst).size = (*src).size;
	(*dst).data = malloc((*src).size * sizeof(float));
	memcpy((*dst).data, (*src).data, sizeof(float) * (*src).size);
	return dst;
}

/**
 * @brief test the equalization output when no gain
 *
 */
Test(idk, equalize_no_gain)
{
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
		signal_new(1.0f, 250, 44100, 100),
		signal_new(1.0f, 2000, 44100, 100),
		signal_new(1.0f, 5000, 44100, 100),
		signal_new(1.0f, 10000, 44100, 100),
	};
	signal_t *amplified_signals[NFILT] = {
		signal_new(1.0f, 250, 44100, 100),
		signal_new(1.0f, 2000, 44100, 100),
		signal_new(1.0f, 5000, 44100, 100),
		signal_new(1.0f, 10000, 44100, 100),
	};

	equalizer_init(44100);
	for (int i = 0; i < NFILT; i++)
	{
		equalizer_equalize((*amplified_signals[i]).data, (*amplified_signals[i]).size);
		// I expect everything is almost(noise) untouched
		for (int k = 0; k < 4410; k++)
		{
		    cr_assert_float_eq((*amplified_signals[i]).data[k], (*source_signals[i]).data[k], 0.1,
		    "signals must be equal");
		}
	}
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

Test(idk, equalize_modify)
{
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
		signal_new(1, 250, 44100, 100),
		signal_new(1, 2000, 44100, 100),
		signal_new(1, 5000, 44100, 100),
		signal_new(1, 10000, 44100, 100),
	};
	signal_t *amplified_signals[NFILT] = {
		signal_new(1, 250, 44100, 100),
		signal_new(1, 2000, 44100, 100),
		signal_new(1, 5000, 44100, 100),
		signal_new(1, 10000, 44100, 100),
	};

	equalizer_init(44100);

	for (int i = 0; i < NFILT; i++)
	{
		equalizer_set_gain(i, MAX_GAIN);
	}
	for (int i = 0; i < NFILT; i++)
	{
		equalizer_equalize((*amplified_signals[i]).data, (*amplified_signals[i]).size);
		// I expect everything is modified
		cr_assert_arr_neq((*amplified_signals[i]).data, (*source_signals[i]).data,
								(*amplified_signals[i]).size * sizeof(float),
								"amplified signal and source signal must be different");
	}
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

Test(idk, band_isolation)
{
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
		signal_new(1, 250, 44100, 100),
		signal_new(1, 2000, 44100, 100),
		signal_new(1, 5000, 44100, 100),
		signal_new(1, 10000, 44100, 100),
	};
	signal_t *amplified_signals[NFILT] = {
		signal_new(1, 250, 44100, 100),
		signal_new(1, 2000, 44100, 100),
		signal_new(1, 5000, 44100, 100),
		signal_new(1, 10000, 44100, 100),
	};

	equalizer_init(44100);

	for (int i = 0; i < NFILT; i++)
	{
		for (int j = 0; j < NFILT; j++)
		{
			if (i == j)
				equalizer_set_gain(j, 0);
			else
				equalizer_set_gain(j, MAX_GAIN);
		}
		equalizer_equalize((*amplified_signals[i]).data, (*amplified_signals[i]).size);
		// I expect the signal will not be touched so much by the equalization
		for (int k = 0; k < (*amplified_signals[i]).size; k++)
		{
			cr_assert_float_eq((*amplified_signals[i]).data[k], (*source_signals[i]).data[k], 10.0f,
			        "amplified signal and source signal must be equal");
		}
	}
	// free memory
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

Test(idk, band_gain)
{
	int i;
	char path[256], filename[100];
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
		signal_new(1.0f, 300, 44100, 100),
		signal_new(1.0f, 2000, 44100, 100),
		signal_new(1.0f, 5000, 44100, 100),
		signal_new(1.0f, 10000, 44100, 100),
	};
	signal_t *amplified_signals[NFILT] = {
		signal_new(1.0f, 300, 44100, 100),
		signal_new(1.0f, 2000, 44100, 100),
		signal_new(1.0f, 5000, 44100, 100),
		signal_new(1.0f, 10000, 44100, 100),
	};
	signal_t *isolated_signals[NFILT] = {
		signal_new(1.0f, 300, 44100, 100),
		signal_new(1.0f, 2000, 44100, 100),
		signal_new(1.0f, 5000, 44100, 100),
		signal_new(1.0f, 10000, 44100, 100),
	};
	FILE *test_results_files[NFILT];
	for (i = 0; i < NFILT; i++)
	{
		strcpy(path, TEST_RESULTS_DIR);
		sprintf(filename, "signal_of_%d_Hz.dat", (*source_signals[i]).signal_freq);
		strcat(path, filename);

		test_results_files[i] = fopen(path, "w+");
		if (test_results_files[i] == NULL)
		{
			perror("creating the file ");
			exit(EXIT_FAILURE);
		}
	}

	equalizer_init(44100);

	for (int i = 0; i < NFILT; i++)
	{
		// set all filters gain to 0, except for the one that should
		// act on this signal setted with max gain (AMPLIFY MY SIGNAL)
		for (int j = 0; j < NFILT; j++)
		{
			if (i == j)
				equalizer_set_gain(j, MAX_GAIN);
			else
				equalizer_set_gain(j, 0);
		}
		equalizer_equalize((*amplified_signals[i]).data, (*amplified_signals[i]).size);
		// set all filters gain to MAX, except for the one that should
		// act on this signal setted with 0 gain
		for (int j = 0; j < NFILT; j++)
		{
			if (i == j)
				equalizer_set_gain(j, 0);
			else
				equalizer_set_gain(j, MAX_GAIN);
		}
		equalizer_equalize((*isolated_signals[i]).data, (*isolated_signals[i]).size);
		// I expect the amplified signal is always greated than the isolated
		for (int k = 0; k < (*amplified_signals[i]).size; k++)
		{
			fprintf(test_results_files[i], "%f\t%f\t%f\n",
					(*source_signals[i]).data[k],
					(*isolated_signals[i]).data[k],
					(*amplified_signals[i]).data[k]);
			// look at these signals in a plot
		}
	}
	// free memory
	for (int i = 0; i < 1; i++)
	{
		fclose(test_results_files[i]);
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
		signal_delete(isolated_signals[i]);
	}
}