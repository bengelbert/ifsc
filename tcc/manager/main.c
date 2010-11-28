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
    guint verbose;
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
    {"verbose", 'v', 0, G_OPTION_ARG_INT, &app.cmdline.verbose, "Verbose level!", NULL},
    {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL}
};

/******************************************************************************/
/*
 * App function definitions
 */
static void
app_init_cmdline(gint * argc, gchar *** argv)
{
    GOptionContext *    context     = NULL;
    GError *            error       = NULL;
    
    context = g_option_context_new("- Server application");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, argc, argv, &error)) {
        g_error("Option parser failed: %s", error->message);
    }
    
    return;
}

/******************************************************************************/

static void
app_start_server(void)
{
    gsm02_async_queue_init();
    mcu_async_queue_init();

    service_socket_add(50000, G_CALLBACK(mcu_connect_handler), NULL);
    service_socket_add(50040, G_CALLBACK(gsm02_connect_handler), NULL);
    service_socket_add(50002, G_CALLBACK(script_connect_handler), NULL);
}

/******************************************************************************/
/*
 * Main program
 */
int main(int argc, char * argv[])
{
    g_type_init();
    g_thread_init(NULL);

    app_init_cmdline(&argc, &argv);

    log_set_level("MCU", app.cmdline.verbose);
    log_set_level("GSM02", app.cmdline.verbose);
    log_set_level("SCRIPT", app.cmdline.verbose);
    log_set_level("SERVICE", app.cmdline.verbose);

    app_start_server();

    g_main_loop_run(g_main_loop_new(NULL, TRUE));

    return (EXIT_SUCCESS);
}
