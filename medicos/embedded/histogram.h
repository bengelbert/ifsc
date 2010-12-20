/**
 * @file    histogram.h
 */

#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H
#ifdef	__cplusplus
extern "C" {
#endif

#include "wrapper.h"

#define HISTOGRAM(obj)  ((histogram_t *) obj)

typedef struct _histogram   histogram_t;

/**
 * 
 * @param t
 * @param samples
 * @param n_samples
 */
void
histogram_do(histogram_t *t,
        float *samples,
        size_t n_samples);

/**
 * 
 * @return
 */
histogram_t *
histogram_get_type(void);

/**
 * 
 * @param t
 */
void
histogram_print(histogram_t *t);

/**
 * 
 * @param t
 * @param val
 */
void
histogram_set_n_baselines(histogram_t *t,
        uint16_t val);

/**
 * 
 * @param t
 * @param val
 */
void
histogram_set_precision(histogram_t *t,
        uint16_t val);

#ifdef	__cplusplus
}
#endif

#endif /* _HISTOGRAM_H */
