#include "wrapper.h"

#define HISTOGRAM_BASELINE_MAX  256

#define HISTOGRAM_MAX(a,b) ((a > b) ? a : b)
#define HISTOGRAM_MIN(a,b) ((a < b) ? a : b)

struct _histogram {
    int32_t min;
    uint16_t baseline;
    uint16_t n_baselines;
    uint16_t precision;
    uint8_t histogram[HISTOGRAM_BASELINE_MAX];
};

/******************************************************************************/

void 
histogram_do(histogram_t *t,
        float *samples,
        size_t n_samples)
{
    int32_t max = 0;
    uint32_t offset = 0;
    uint32_t range = 0;
    uint16_t i;

    for (i = 0; i < n_samples; i++) {
        max = HISTOGRAM_MAX(max, pgm_read_float(&samples[i]) * t->precision);
        t->min = HISTOGRAM_MIN(t->min, pgm_read_float(&samples[i]) * t->precision);
    }

    range = max - t->min;
    t->baseline = range / t->n_baselines;

    for (i = 0; i < n_samples; i++) {
        offset = ((pgm_read_float(&samples[i]) * t->precision) - t->min) / t->baseline;
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
    uint16_t i, j;

    for (i = 0; i < t->n_baselines; i++) {
        rprintfFloat(4, ((double) t->min + (double) (i * t->baseline)) / t->precision);
        rprintf(" to ");
        rprintfFloat(4, ((double) t->min + (double) ((i+1) * t->baseline)) / t->precision);
        rprintf(": ");

        for (j = 0; j < t->histogram[i]; j++) {
            rprintf("*");
        }
        
        rprintf("\n");
    }
}

/******************************************************************************/

void
histogram_set_n_baselines(histogram_t *t,
        uint16_t val)
{
    if (val < HISTOGRAM_BASELINE_MAX) {
        t->n_baselines = val;
    }
}

/******************************************************************************/

void
histogram_set_precision(histogram_t *t,
        uint16_t val)
{
    t->precision = val;
}

/******************************************************************************/
