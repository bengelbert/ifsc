/* 
 * File:   movavg.h
 * Author: engelbert
 *
 * Created on 19 de Dezembro de 2010, 21:13
 */

#ifndef MOVAVG_H
#define	MOVAVG_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * 
 * @param t
 * @param samples
 * @param n_samples
 */
void
movavg_do(float *samples,
        uint16_t n_samples);

#ifdef	__cplusplus
}
#endif

#endif	/* MOVAVG_H */

