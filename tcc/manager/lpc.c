#include "wrapper.h"

/*
 * Constants
 */
#define LPC_BUFFER_LEN  128
#define LPC_SERVER_ID   0x00
#define LPC_VERSION     0x00

#define LPC_CMD_PING    0x01

/*
 * Type definitions
 */
typedef struct _LpcHeader  LpcHeader;

struct _LpcHeader {
    guint16 m_num_bytes;
    guint16 m_version;
    guint16 m_id;
    guint8  m_cmd;
} PACKED;

/*
 * Macros
 */
#define LPC_HEADER_LEN (sizeof (LpcHeader))

/*
 * Private prototype functions 
 */
static void     lpc_client_cb(  GConn *         __conn,
                                GConnEvent *    __event,
                                gpointer        __user_data);

static gint     lpc_make_header(gchar * __buffer,
                                gint    __cmd,
                                gint    __payload_len);

static void     lpc_print_dump( gchar * __buffer,
                                gint    __len);

static gboolean lpc_send_ping(  gpointer __user_data);

static void     lpc_write(  GConn * __conn,
                            gchar * __buffer,
                            gint    __len);

/******************************************************************************/


/*
 * Functions definitions
 */
static void
lpc_client_cb(  GConn *         __conn,
                GConnEvent *    __event,
                gpointer        __user_data)
{
    g_assert(__user_data);

    switch (__event->type) {
    case GNET_CONN_READ:
        __event->buffer[__event->length - 1] = '\n';
        gnet_conn_write     (__conn, __event->buffer, __event->length);
        gnet_conn_readline  (__conn);

        break;

    case GNET_CONN_WRITE:
        break;

    case GNET_CONN_CLOSE:
    case GNET_CONN_TIMEOUT:
    case GNET_CONN_ERROR:
        gnet_conn_delete(__conn);
        
        break;

    default:
        g_assert_not_reached();
        
    }
}

/******************************************************************************/

static gint
lpc_make_header(gchar * __buffer,
                gint    __cmd,
                gint    __payload_len)
{
    gint pack_len   = 0;

    pack_len = LPC_HEADER_LEN + __payload_len;

    gnet_pack(  "hhbh",
                __buffer,
                LPC_HEADER_LEN,
                g_htons(pack_len),
                g_htons(LPC_VERSION),
                g_htons(LPC_SERVER_ID),
                __cmd);

    return pack_len;
}

/******************************************************************************/

static void
lpc_print_dump( gchar * __buffer,
                gint    __len)
{
    gint i = 0;

    g_debug("Dump:");
    for (i = 0; i < __len; i++) {
        if (!(i % 20))
            g_print("\n\t");
        g_print("%02X ", (guchar) __buffer[i]);
    }
    g_print("\n");

    return;
}

/******************************************************************************/

static gboolean
lpc_send_ping(gpointer __user_data)
{
    gint    pack_len    = 0;
    GConn * p_conn      = NULL;
    gchar   v_buffer[LPC_BUFFER_LEN];

    g_assert(__user_data != NULL);
    
    p_conn = (GConn *) __user_data;

    pack_len = lpc_make_header(v_buffer, LPC_CMD_PING, 0);

    lpc_write(p_conn, v_buffer, pack_len);

    return true;
}

/******************************************************************************/

void
lpc_server_cb(  GServer *   __server,
                GConn *     __conn,
                gpointer    __user_data)
{
    if (__conn) {
        gnet_conn_set_callback      (__conn, lpc_client_cb, __user_data);
        gnet_conn_set_watch_error   (__conn, true);
        gnet_conn_readline          (__conn);

        g_timeout_add(1000, lpc_send_ping, __conn);
    } else {
        gnet_server_delete(__server);
        exit(EXIT_FAILURE);
    }
}

/******************************************************************************/

static void
lpc_write(  GConn * __conn,
            gchar * __buffer,
            gint    __len)
{
    g_assert(__buffer   != NULL);
    g_assert(__conn     != NULL);

    lpc_print_dump(__buffer, __len);
    gnet_conn_write(__conn, __buffer, __len);

    return;
}

/******************************************************************************/

