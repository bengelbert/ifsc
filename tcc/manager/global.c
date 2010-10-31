#include "wrapper.h"

#define GLOBAL_FILE "config.ini"

#define GLOBAL_GROUP_CLIENT "CLIENT"
#define GLOBAL_GROUP_LOG    "LOG"

#define GLOBAL_KEY_LEVEL_CLIENT     "LEVEL_CLIENT"
#define GLOBAL_KEY_LEVEL_COMMON     "LEVEL_COMMON"
#define GLOBAL_KEY_LEVEL_SERVER     "LEVEL_SERVER"
#define GLOBAL_KEY_PING_INTERVAL    "PING_INTERVAL"
#define GLOBAL_KEY_PORT             "PORT"

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct _GlobalClient    GlobalClient;
typedef struct _GlobalKeyFile   GlobalKeyFile;

/******************************************************************************/

struct _Global {
    GlobalClient * client;
};

/******************************************************************************/

struct _GlobalClient {
    guint ping_interval;
    guint port;
};

/******************************************************************************/

struct _GlobalKeyFile {
    gint    id;
    gchar * group;
    gchar * key;
};

/******************************************************************************/

enum {
    GLOBAL_NULL,
    GLOBAL_LOG_LEVEL_CLIENT,
    GLOBAL_LOG_LEVEL_COMMON,
    GLOBAL_LOG_LEVEL_SERVER,
    GLOBAL_CLIENT_PING_INTERVAL,
    GLOBAL_CLIENT_PORT,
};

/*
 * Function prototypes
 */
static void global_load_from_file           (Global * __global, gchar * __file);
static void global_set_client_ping_interval (Global * __global, guint __interval);
static void global_set_client_port          (Global * __global, guint __port);

/******************************************************************************/

/*
 * Private variables
 */
static struct _GlobalKeyFile keyfile[] = {
    {GLOBAL_LOG_LEVEL_CLIENT,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_CLIENT},
    {GLOBAL_LOG_LEVEL_COMMON,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_COMMON},
    {GLOBAL_LOG_LEVEL_SERVER,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_SERVER},
    {GLOBAL_CLIENT_PING_INTERVAL,GLOBAL_GROUP_CLIENT, GLOBAL_KEY_PING_INTERVAL},
    {GLOBAL_CLIENT_PORT,GLOBAL_GROUP_CLIENT, GLOBAL_KEY_PORT},
    {GLOBAL_NULL, NULL, NULL}
};

/******************************************************************************/

/*
 * Function definitions
 */
guint
global_get_client_ping_interval(Global * __global)
{
    g_assert(__global);
    
    debug("GLOBAL", "([%p])", __global);

    return __global->client->ping_interval;
}

/******************************************************************************/

guint
global_get_client_port(Global * __global)
{
    g_assert(__global);

    debug("GLOBAL", "([%p])", __global);

    return __global->client->port;
}

/******************************************************************************/

Global *
global_new(void)
{
    Global * global   = NULL;

    global = g_new0(Global, 1);

    global->client = g_new0(GlobalClient, 1);

    global_load_from_file(global, GLOBAL_FILE);

    return global;
}

/******************************************************************************/

void
global_load_from_file(Global *  __global,
                    gchar *     __file)
{
    gchar *         data    = NULL;
    GError *        error   = NULL;
    guint           i       = 0;
    GKeyFileFlags   flags   = G_KEY_FILE_NONE;
    GKeyFile *      key     = NULL;

    g_assert(__global);

    debug("GLOBAL", "([%p], [%s])", __global, __file);

    flags = G_KEY_FILE_NONE |
            G_KEY_FILE_KEEP_COMMENTS |
            G_KEY_FILE_KEEP_TRANSLATIONS;

    key = g_key_file_new();
    g_assert(key);

    if(!g_key_file_load_from_file(key, __file, flags, &error))
        error("GLOBAL", "%s [%s]", error->message, __file);

    for (i = 0; keyfile[i].key != NULL; i++) {
        if ((data = g_key_file_get_value(key, keyfile[i].group, keyfile[i].key, &error)) == NULL)
            error("GLOBAL", "%s", error->message);

        switch (keyfile[i].id) {
        case GLOBAL_LOG_LEVEL_CLIENT:       log_set_level("CLIENT", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_LOG_LEVEL_COMMON:       log_set_level("COMMON", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_LOG_LEVEL_SERVER:       log_set_level("SERVER", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_CLIENT_PING_INTERVAL:   global_set_client_ping_interval(__global, g_ascii_strtoll(data, NULL, 10)); break;
        case GLOBAL_CLIENT_PORT:            global_set_client_port(__global, g_ascii_strtoll(data, NULL, 10));          break;
        }
    }
    
    g_key_file_free(key);
}

/******************************************************************************/

static void
global_set_client_ping_interval(Global *    __global,
                                guint       __interval)
{
    g_assert(__global);

    debug("GLOBAL", "([%p], [%d])", __global, __interval);

    __global->client->ping_interval = __interval;
}

/******************************************************************************/

static void
global_set_client_port(Global * __global,
                    guint       __port)
{
    g_assert(__global);

    debug("GLOBAL", "([%p], [%d])", __global, __port);

    __global->client->port = __port;
}

/******************************************************************************/
