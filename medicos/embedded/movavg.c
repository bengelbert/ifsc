#include "wrapper.h"

#define MOVAVG_WINDOW   101

/******************************************************************************/

void
movavg_do(float *samples,
        uint16_t n_samples)
{
    uint16_t i, p, q;
    double acc = 0;
    double result = 0;

    for (i = 0; i < MOVAVG_WINDOW; i++) {
        acc += pgm_read_float(&samples[i]);
    }

    p = (MOVAVG_WINDOW - 1) / 2;
    q = p + 1;

    for (i = q; i < (n_samples - p); i++) {
        acc = acc + pgm_read_float(&samples[i+p]) - pgm_read_float(&samples[i-q]);
        result = fma(acc, 1.0/MOVAVG_WINDOW, 0);
    }
}

/******************************************************************************/
