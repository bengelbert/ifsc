/******************************************************************************/
/*
 * Application include files
 */
#include "wrapper.h"

/******************************************************************************/
/*
 * Function definitions
 */
void
common_print_dump(gchar *   __buffer,
                guint       __len)
{
    gchar * buffer  = NULL;
    guint   i       = 0;
    
    debug("COMMON", "([%p], [%d])", __buffer, __len);

    buffer = g_new0(gchar, __len * 3);

    for (i = 0; i < __len; i++)
        snprintf(&buffer[i*3], __len, "%02X ", __buffer[i]);
    
    info("COMMON", "Dump: %s", buffer);

    g_free(buffer);
}

/******************************************************************************/

guint
common_timeout_add(gchar *  __name,
                guint       __interval,
                GSourceFunc __func,
                gpointer    __data)
{
    guint   id      = 0;
    gchar * status  = "fail";

    id = g_timeout_add(__interval, __func, __data);

    if (id > 0) {
        status = "pass";
    }

    debug("COMMON",
            "() %s: [%d] = ([%d], [%p], [%p]) - %s!",
            __name,
            id,
            __interval,
            __func,
            __data,
            status);

    return id;
}

/******************************************************************************/


