#include "wrapper.h"

#define HISTOGRAM_BASELINE_MAX  1024

#define HISTOGRAM_MAX(a,b) ((a > b) ? a : b)
#define HISTOGRAM_MIN(a,b) ((a < b) ? a : b)

struct _histogram {
    int32_t min;
    uint32_t baseline;
    uint32_t n_baselines;
    uint32_t precision;
    uint8_t histogram[HISTOGRAM_BASELINE_MAX];
};

/******************************************************************************/

void 
histogram_calc(histogram_t *t,
        float *samples,
        size_t n_samples)
{
    int32_t max = 0;
    uint32_t offset = 0;
    uint32_t range = 0;
    uint32_t i;

    for (i = 0; i < n_samples; i++) {
        max = HISTOGRAM_MAX(max, samples[i] * t->precision);
        t->min = HISTOGRAM_MIN(t->min, samples[i] * t->precision);
    }

    range = max - t->min;
    t->baseline = range / t->n_baselines;

    for (i = 0; i < n_samples; i++) {
        offset = ((samples[i] * t->precision) - t->min) / t->baseline;
        if (offset < t->n_baselines)
            t->histogram[offset]++;
    }
}

/******************************************************************************/

histogram_t *
histogram_get_type(void)
{
    static int init = 0;
    static histogram_t histogram;

    if (init == 0) {
        memset(&histogram, 0, sizeof(histogram_t));
        init = 1;
    }

    return &histogram;
}

/******************************************************************************/

void
histogram_print(histogram_t *t)
{
    uint32_t i, j;

    printf("   Baselines    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10|\n");
    printf("----------------+---+---+---+---+---+---+---+---+---+---+\n");
    for (i = 0; i < t->n_baselines; i++) {
        printf("%.1f to %.1f\t|", ((double) t->min + (double) (i * t->baseline)) / t->precision,
                ((double) t->min + (double) ((i+1) * t->baseline)) / t->precision);
        for (j = 0; j < t->histogram[i]; j++) {
            printf(" * |");
        }
        printf("\n");
    }
}

/******************************************************************************/

void
histogram_set_n_baselines(histogram_t *t,
        uint32_t val)
{
    if (val < HISTOGRAM_BASELINE_MAX) {
        t->n_baselines = val;
    }
}

/******************************************************************************/

void
histogram_set_precision(histogram_t *t,
        uint32_t val)
{
    t->precision = val;
}

/******************************************************************************/
