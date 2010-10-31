/* 
 * File:   main.c
 * Author: bruno
 *
 * Created on 29 de Julho de 2010, 22:11
 */

#include "wrapper.h"

/*
 * App type definitions
 */
typedef struct app_s                app_t;
typedef struct app_entries_data_s   app_entries_data_t;

/******************************************************************************/

struct app_entries_data_s {
    guint client;
    guint server;
};

/******************************************************************************/

struct app_s {
    app_entries_data_t  cmdline;
};

/******************************************************************************/
/*
 * App variables 
 */
static app_t app;
static GOptionEntry entries[] = {
    {"client", 'c', 0, G_OPTION_ARG_NONE, &app.cmdline.client, "Start client!", NULL},
    {"server", 's', 0, G_OPTION_ARG_NONE, &app.cmdline.server, "Start server!", NULL},
    {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL}
};

/******************************************************************************/
/*
 * App function definitions
 */
static void
app_exit(gchar * name)
{
    g_print("Use: %s [OPTION...]\n", name);
    g_print("Try \"%s --help\" for more informations.\n", name);
    exit(EXIT_FAILURE);
}

/******************************************************************************/

static void
app_init_cmdline(gint * argc, gchar *** argv)
{
    GOptionContext *    context     = NULL;
    GError *            error       = NULL;
    
    context = g_option_context_new("- Server/client application");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, argc, argv, &error)) {
        g_error("Option parser failed: %s", error->message);
    }
    
    return;
}

/******************************************************************************/

static void
app_start_client(void)
{
    g_message("Starting app client!");

    client_init(global_new());

    return;
}

/******************************************************************************/

static void
app_start_server(void)
{
    g_message("Starting app server!");

    server_init(global_new());
    
    return;
}

/******************************************************************************/
/*
 * Main program
 */
int main(int argc, char * argv[])
{
    GMainLoop * main_loop = NULL;

    g_thread_init(NULL);

    main_loop = g_main_loop_new(NULL, false);

    app_init_cmdline(&argc, &argv);
    
    if (app.cmdline.server == true) {
        app_start_server();
    } else if (app.cmdline.client == true) {
        app_start_client();
    } else {
       app_exit(argv[0]);
    }

    g_main_loop_run(main_loop);

    return (EXIT_SUCCESS);
}
