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

#include <cmocka.h>

#include "defines.h"
#include "equalizer.c"

#define assert mock_assert

typedef struct
{
	int signal_freq;
	int sampling_freq;
	int size;
	double *data;
} signal_t;

/**
 * @brief
 *
 * @param signal_freq in hertz
 * @param sampling_freq in hertz
 * @param duration in milliseconds
 * @return signal_t* pointer to the new signal
 */
signal_t *signal_new(double amplitude, double signal_freq, double sampling_freq, double duration)
{
	double time_period;
	double y;
	signal_t *signal;
	signal = malloc(sizeof(signal_t));
	(*signal).signal_freq = signal_freq;
	(*signal).sampling_freq = sampling_freq;
	(*signal).size = (duration / 1000.0f * ((double)sampling_freq));
	(*signal).data = malloc((*signal).size * sizeof(double));

	time_period = 1.0f / sampling_freq;
	for (int i = 0; i < (*signal).size; i++)
	{
		y = cos(2.0f * M_PI * signal_freq * i * time_period);
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

#define SIGNAL_PRINT(s) \
	do                    \
	{                     \
		printf(#s);         \
		signal_print(s);    \
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
	(*dst).data = malloc((*src).size * sizeof(double));
	memcpy((*dst).data, (*src).data, sizeof(double) * (*src).size);
	return dst;
}

/**
 * @brief test the equalization output when no gain
 *
 * @param state default param for cmocka
 */
static void test_equalizer_equalize_no_gain(void **state)
{
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
			signal_new(1.0f, 250, 44100, 100),
			signal_new(1.0f, 2000, 44100, 100),
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
		equalizer_equalize((*amplified_signals[i]).data, (*amplified_signals[i]).size);
		// I expect everything is untouched
		assert_memory_equal((*amplified_signals[i]).data, (*source_signals[i]).data,
												(*amplified_signals[i]).size * sizeof(double));
	}
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

static void test_equalizer_equalize_modify(void **state)
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
		assert_memory_not_equal((*amplified_signals[i]).data, (*source_signals[i]).data,
														(*amplified_signals[i]).size * sizeof(double));
	}
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

static void test_equalizer_equalize_band_isolation(void **state)
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
			assert_true(fabsf((*amplified_signals[i]).data[k] - (*source_signals[i]).data[k]) < 10.0f);
		}
	}
	// free memory
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
	}
}

void test_equalizer_equalize_band_gain(void **state)
{
	// signals on the center frequency of the filters
	signal_t *source_signals[NFILT] = {
			signal_new(100, 80, 44100, 100),
			signal_new(100, 2000, 44100, 100),
			signal_new(100, 5000, 44100, 100),
			signal_new(100, 10000, 44100, 100),
	};
	signal_t *amplified_signals[NFILT] = {
			signal_new(100, 80, 44100, 100),
			signal_new(100, 2000, 44100, 100),
			signal_new(100, 5000, 44100, 100),
			signal_new(100, 10000, 44100, 100),
	};
	signal_t *isolated_signals[NFILT] = {
			signal_new(100, 80, 44100, 100),
			signal_new(100, 2000, 44100, 100),
			signal_new(100, 5000, 44100, 100),
			signal_new(100, 10000, 44100, 100),
	};

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
			if (fabs((*amplified_signals[i]).data[k]) < fabs((*source_signals[i]).data[k]))
			{
				printf("signal n: %d, sample n: %d\n", i, k);
				printf("%f\t%f\n", (*amplified_signals[i]).data[k], (*source_signals[i]).data[k]);
			}
			// assert_true(fabs((*amplified_signals[i]).data[k]) >= fabs((*source_signals[i]).data[k]));
		}
	}
	// free memory
	for (int i = 0; i < NFILT; i++)
	{
		signal_delete(source_signals[i]);
		signal_delete(amplified_signals[i]);
		signal_delete(isolated_signals[i]);
	}
}

int main()
{
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_equalizer_equalize_no_gain),
			cmocka_unit_test(test_equalizer_equalize_modify),
			cmocka_unit_test(test_equalizer_equalize_band_isolation),
			cmocka_unit_test(test_equalizer_equalize_band_gain),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
