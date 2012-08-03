#include "wrapper.h"

#define GLOBAL_FILE "config.ini"

#define GLOBAL_GROUP_CLIENT "CLIENT"
#define GLOBAL_GROUP_LOG    "LOG"
#define GLOBAL_GROUP_SERVER "SERVER"

#define GLOBAL_KEY_LEVEL_CLIENT     "LEVEL_CLIENT"
#define GLOBAL_KEY_LEVEL_COMMON     "LEVEL_COMMON"
#define GLOBAL_KEY_LEVEL_SERVER     "LEVEL_SERVER"
#define GLOBAL_KEY_PING_INTERVAL    "PING_INTERVAL"
#define GLOBAL_KEY_PORT             "PORT"

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct global_client_s      global_client_t;
typedef struct global_server_s      global_server_t;
typedef struct global_key_file_s    global_key_file_t;

/******************************************************************************/

struct global_s {
    global_client_t * client;
    global_server_t * server;
};

/******************************************************************************/

struct global_client_s {
    guint ping_interval;
    guint port;
};

/******************************************************************************/

struct global_key_file_s {
    gint    id;
    gchar * group;
    gchar * key;
};

/******************************************************************************/

struct global_server_s {
    guint ping_interval;
    guint port;
};

/******************************************************************************/

enum {
    GLOBAL_NULL,
    GLOBAL_LOG_LEVEL_CLIENT,
    GLOBAL_LOG_LEVEL_COMMON,
    GLOBAL_LOG_LEVEL_SERVER,
    GLOBAL_CLIENT_PING_INTERVAL,
    GLOBAL_CLIENT_PORT,
    GLOBAL_SERVER_PING_INTERVAL,
    GLOBAL_SERVER_PORT,
};

/*
 * Function prototypes
 */
static void global_load_from_file           (global_t * global, gchar * file);
static void global_set_client_ping_interval (global_t * global, guint interval);
static void global_set_client_port          (global_t * global, guint port);
static void global_set_server_ping_interval (global_t * global, guint interval);
static void global_set_server_port          (global_t * global, guint port);

/******************************************************************************/

/*
 * Private variables
 */
static struct global_key_file_s keyfile[] = {
    {GLOBAL_LOG_LEVEL_CLIENT,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_CLIENT},
    {GLOBAL_LOG_LEVEL_COMMON,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_COMMON},
    {GLOBAL_LOG_LEVEL_SERVER,GLOBAL_GROUP_LOG, GLOBAL_KEY_LEVEL_SERVER},
    {GLOBAL_CLIENT_PING_INTERVAL,GLOBAL_GROUP_CLIENT, GLOBAL_KEY_PING_INTERVAL},
    {GLOBAL_CLIENT_PORT,GLOBAL_GROUP_CLIENT, GLOBAL_KEY_PORT},
    {GLOBAL_SERVER_PING_INTERVAL,GLOBAL_GROUP_SERVER, GLOBAL_KEY_PING_INTERVAL},
    {GLOBAL_SERVER_PORT,GLOBAL_GROUP_SERVER, GLOBAL_KEY_PORT},
    {GLOBAL_NULL, NULL, NULL}
};

/******************************************************************************/

/*
 * Function definitions
 */
guint
global_get_client_ping_interval(global_t * __global)
{
    g_assert(__global);
    
    debug("GLOBAL", "([%p])", __global);

    return __global->client->ping_interval;
}

/******************************************************************************/

guint
global_get_client_port(global_t * __global)
{
    g_assert(__global);

    debug("GLOBAL", "([%p])", __global);

    return __global->client->port;
}

/******************************************************************************/

guint
global_get_server_ping_interval(global_t * global)
{
    g_assert(global);
    g_assert(global->server);

    debug("GLOBAL", "[%d]", global->server->ping_interval);

    return global->server->ping_interval;
}

/******************************************************************************/

guint
global_get_server_port(global_t * global)
{
    g_assert(global);
    g_assert(global->server);

    debug("GLOBAL", "[%d]", global->server->port);

    return global->server->port;
}

/******************************************************************************/

global_t *
global_new(void)
{
    global_t * global = NULL;

    global = g_new0(global_t, 1);

    global->client = g_new0(global_client_t, 1);
    global->server = g_new0(global_server_t, 1);

    debug("GLOBAL", "global ------------- [%p]", global);
    debug("GLOBAL", "global->client ----- [%p]", global->client);
    debug("GLOBAL", "global->server ----- [%p]", global->server);

    global_load_from_file(global, GLOBAL_FILE);

    return global;
}

/******************************************************************************/

void
global_load_from_file(global_t * global, gchar * file)
{
    gchar *         data    = NULL;
    GError *        error   = NULL;
    guint           i       = 0;
    GKeyFileFlags   flags   = G_KEY_FILE_NONE;
    GKeyFile *      key     = NULL;

    g_assert(file);
    g_assert(global);
    g_assert(global->client);
    g_assert(global->server);

    debug("GLOBAL", "file ----- [%s]", file);

    flags = G_KEY_FILE_NONE | G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

    key = g_key_file_new();
    g_assert(key);

    if(!g_key_file_load_from_file(key, file, flags, &error)) {
        error("GLOBAL", "%s [%s]", error->message, file);
        g_error_free(error);
        error = NULL;
    }

    for (i = 0; keyfile[i].key != NULL; i++) {
        if ((data = g_key_file_get_value(key, keyfile[i].group, keyfile[i].key, &error)) == NULL) {
            error("GLOBAL", "%s", error->message);
            g_error_free(error);
            error = NULL;
        }

        switch (keyfile[i].id) {
        case GLOBAL_LOG_LEVEL_CLIENT:       log_set_level("CLIENT", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_LOG_LEVEL_COMMON:       log_set_level("COMMON", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_LOG_LEVEL_SERVER:       log_set_level("SERVER", g_ascii_strtoll(data, NULL, 10));                   break;
        case GLOBAL_CLIENT_PING_INTERVAL:   global_set_client_ping_interval(global, g_ascii_strtoll(data, NULL, 10));   break;
        case GLOBAL_CLIENT_PORT:            global_set_client_port(global, g_ascii_strtoll(data, NULL, 10));            break;
        case GLOBAL_SERVER_PING_INTERVAL:   global_set_server_ping_interval(global, g_ascii_strtoll(data, NULL, 10));   break;
        case GLOBAL_SERVER_PORT:            global_set_server_port(global, g_ascii_strtoll(data, NULL, 10));            break;
        }
    }
    
    g_key_file_free(key);
}

/******************************************************************************/

static void
global_set_client_ping_interval(global_t * global, guint interval)
{
    g_assert(global);
    g_assert(global->client);

    debug("GLOBAL", "[%d]", interval);

    global->client->ping_interval = interval;
}

/******************************************************************************/

static void
global_set_client_port(global_t * global, guint port)
{
    g_assert(global);
    g_assert(global->client);

    debug("GLOBAL", "[%d]", port);

    global->client->port = port;
}

/******************************************************************************/

static void
global_set_server_ping_interval(global_t * global, guint interval)
{
    g_assert(global);
    g_assert(global->server);

    debug("GLOBAL", "[%d]", interval);

    global->server->ping_interval = interval;
}

/******************************************************************************/

static void
global_set_server_port(global_t * global, guint port)
{
    g_assert(global);
    g_assert(global->server);

    debug("GLOBAL", "[%d]", port);

    global->server->port = port;
}

/******************************************************************************/
