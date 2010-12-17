/**
 * @file    main.c
 */

#include <stdio.h>
#include <stdint.h>

#include "histogram.h"

static float samples[] = {
    -6.0000000e+000,
    -3.9062460e+000,
    -3.9062460e+000,
    3.9062460e+000,
    -9.7656170e+000,
    -9.7656170e+000,
    -7.8124920e+000,
    3.9062470e+000,
    -0.0000000e+000,
    -5.8593700e+000,
    1.1718740e+001,
    1.7578110e+001,
    -9.7656170e+000,
    -1.5624988e+001,
    5.8593690e+000,
    7.8124940e+000,
    1.9531230e+000,
    -0.0000000e+000,
    3.9062460e+000,
    1.7578110e+001,
    1.1718740e+001,
    1.1718740e+001,
    1.3671864e+001,
    5.8593700e+000,
    9.7656160e+000,
    3.9062460e+000,
    -5.8593690e+000,
    -7.8124920e+000,
    -1.9531235e+001,
    -3.9062460e+000,
    -1.5624987e+001,
    -2.9296848e+001,
    -2.5390602e+001,
    -1.1718742e+001,
    -5.8593690e+000,
    -1.7578110e+001,
    -1.1718740e+001,
    -0.0000000e+000,
    -1.9531230e+000,
    -7.8124930e+000,
    -1.3671864e+001,
    -5.8593700e+000,
    -7.8124930e+000,
    -1.9531230e+000,
    -7.8124930e+000,
    -1.9531233e+001,
    -3.9062460e+000,
    -3.9062460e+000,
    -9.7656160e+000,
    -7.8124930e+000,
    -7.8124930e+000,
    1.9531230e+000
};

#define NUMBER_OF_SAMPLES   ((size_t) sizeof (samples) / sizeof (float))

int
main(void)
{
    histogram_t *histogram = NULL;
    
    histogram = histogram_get_type();

    histogram_set_n_baselines(histogram, 24);
    histogram_set_precision(histogram, 10000);
    histogram_calc(histogram, samples, NUMBER_OF_SAMPLES);
    histogram_print(histogram);

    return 0;
}
