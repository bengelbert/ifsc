#include "wrapper.h"

/*
 * Function definitions
 */
static void
log_dummy_handler(void)
{
    
}

/******************************************************************************/

void
log_set_level(gchar *   __domain,
                guint   __level)
{
    g_assert(__domain);

    debug("LOG", "([%s], [%d])", __domain, __level);

    if (__level < 7) g_log_set_handler(__domain, G_LOG_LEVEL_DEBUG, (GLogFunc)log_dummy_handler, NULL);
    if (__level < 6) g_log_set_handler(__domain, G_LOG_LEVEL_INFO, (GLogFunc)log_dummy_handler, NULL);
    if (__level < 5) g_log_set_handler(__domain, G_LOG_LEVEL_MESSAGE, (GLogFunc)log_dummy_handler, NULL);
    if (__level < 4) g_log_set_handler(__domain, G_LOG_LEVEL_WARNING, (GLogFunc)log_dummy_handler, NULL);
    if (__level < 3) g_log_set_handler(__domain, G_LOG_LEVEL_CRITICAL, (GLogFunc)log_dummy_handler, NULL);
    if (__level < 2) g_log_set_handler(__domain, G_LOG_LEVEL_ERROR, (GLogFunc)log_dummy_handler, NULL);
}

/******************************************************************************/
