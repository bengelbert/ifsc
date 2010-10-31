/* 
 * File:   log.h
 * Author: bruno
 *
 * Created on 5 de Setembro de 2010, 18:58
 */

#ifndef LOG_H
#define	LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "wrapper.h"
    
#define error(domain, format, ...)      (g_log(domain, G_LOG_LEVEL_ERROR, format, ## __VA_ARGS__))
#define critical(domain, format, ...)   (g_log(domain, G_LOG_LEVEL_CRITICAL, format, ## __VA_ARGS__))
#define warning(domain, format, ...)    (g_log(domain, G_LOG_LEVEL_WARNING, format, ## __VA_ARGS__))
#define message(domain, format, ...)    (g_log(domain, G_LOG_LEVEL_MESSAGE, format, ## __VA_ARGS__))
#define info(domain, format, ...)       (g_log(domain, G_LOG_LEVEL_INFO, format, ## __VA_ARGS__))
#define debug(domain, format, ...)      (g_log(domain, G_LOG_LEVEL_DEBUG, "%s" format, __FUNCTION__, ## __VA_ARGS__))

void    log_set_level(gchar * __domain, guint __level);

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

