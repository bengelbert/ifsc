#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "dgt.h"
#include "FreeRTOS.h"
#include "lcd16x2.h"
#include "psock.h"
#include "pt.h"
#include "queue.h"
#include "uip.h"
#include "uip_arch.h"
#include "task.h"
#include "uip-conf.h"
#include "rtos/portable/GCC/CORTUS_APS3/portmacro.h"

/******************************************************************************/
/*
 * Constants
 */
#define CLIENT_BUFFER_LEN 128
#define CLIENT_HEADER_LEN 15

#define CLIENT_CMD_SEND_KEEP_ALIVE  0x00
#define CLIENT_CMD_SEND_LED_STATE   0x01

#define CLIENT_CMD_RECV_KEEP_ALIVE  0x20
#define CLIENT_CMD_RECV_DTMF        0x21

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct client_message_s client_message_t;
typedef struct client_real_s client_real_t;

/******************************************************************************/

struct client_message_s {
    uint16_t n_bytes;
    uint16_t version;
    uint8_t cmd;
    uint32_t time_event;
    uint32_t time_send;
    uint16_t id;
    uint8_t payload[CLIENT_BUFFER_LEN];
} PACK_STRUCT_END;

/******************************************************************************/

struct client_real_s {
    void(*connect)(client_t *this);

    lcd16x2_t *lcd;
    uip_ipaddr_t addr_local;
    uip_ipaddr_t addr_serv;

    void(*closed)(client_t * client);
    
    void(*connected)(client_t * client);

    void(*led_off)(void);

    void(*led_on)(void);

    uint8_t(*message_get_cmd)(void *header);

    uint8_t(*message_get_n_bytes)(void *header);

    void(*message_handler)(client_t *client,
            void *data);

    size_t (*message_make_led_state)(client_real_t *this,
        void *data);

    void(*message_make_header)(uint8_t cmd,
            void *data,
            size_t payload_len);

    uint8_t *(*message_pack_16)(uint8_t *pack,
            uint32_t value);

    uint16_t(*message_unpack_16)(uint8_t *pack);

    void(*newdata)(client_t * client);

    void(*recv_dtmf)(client_t *client,
        client_message_t *message);

    void(*send_keep_alive)(client_t *client);

    void(*send_led_state)(client_real_t *this);

};

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 *
 * @param client
 */
static void
client_connect(client_t *client);

/**
 * 
 * @param header
 * @return
 */
static uint8_t
client_message_get_cmd(void *data)
{
    client_message_t *message = data;

    return message->cmd;
}

/**
 * 
 * @param header
 * @return
 */
static uint8_t
client_message_get_n_bytes(void *data)
{
    client_message_t *message = data;

    return HTONS(message->n_bytes);
}

/**
 * 
 * @param this
 * @param data
 * @return
 */
static size_t
client_message_make_led_state(client_real_t *this,
        void *data);

/**
 * 
 * @param cmd
 * @param data
 * @param payload_len
 */
static void
client_message_make_header(uint8_t cmd,
        void *data,
        size_t payload_len);

/**
 * 
 * @param pack
 * @param value
 * @return
 */
static uint8_t *
client_message_pack_16(uint8_t *pack,
        uint32_t value);

/**
 * 
 * @param pack
 * @return
 */
static uint16_t
client_message_unpack_16(uint8_t *pack);

/**
 * 
 */
static void
client_led_off(void);

/**
 *
 */
static void
client_led_on(void);

/**
 *
 * @param client
 * @param data
 */
static void
client_message_handler(client_t *client,
        void *data);

/**
 * 
 * @param client
 * @param message
 */
static void
client_recv_dtmf(client_t *client,
        client_message_t *message);

/**
 * 
 * @param this
 */
static void
client_send_led_state(client_real_t *this);

/**
 * 
 * @param client
 */
static void
client_send_keep_alive(client_t *client);

/**
 * 
 * @param self
 */
static void
client_uip_closed(client_t *client);

/**
 *
 * @param self
 */
static void
client_uip_connected(client_t *client);

/**
 *
 * @param self
 */
static void
client_uip_newdata(client_t *client);

/******************************************************************************/

/*
 * Function definitions
 */
static void
client_connect(client_t *client)
{
    client_real_t *this = (client_real_t *) client;

    lcd16x2_async_queue_push(this->lcd,
            "Try to connect..",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);

    uip_connect(&(this->addr_serv), HTONS(50000));
}

/******************************************************************************/

void
client_appcall(void *data)
{
    client_real_t *obj = data;

    if (uip_connected()) {

        obj->connected(data);

    } else if (uip_closed() || uip_aborted() || uip_timedout()) {

        obj->closed(data);

    } else if (uip_newdata()) {

        obj->newdata(data);
        
    } else if (uip_rexmit() || uip_acked() || uip_poll()) {

    }
}

/******************************************************************************/

static void
client_led_off(void)
{
    FIO2CLR0 |= 1 << 3;
}

/******************************************************************************/

static void
client_led_on(void)
{
    FIO2SET0 |= 1 << 3;
}

/******************************************************************************/

static void
client_message_handler(client_t *client,
        void *data)
{
    client_real_t *this = (client_real_t *) client;

    switch (this->message_get_cmd(data)) {
    case CLIENT_CMD_RECV_DTMF:
        this->recv_dtmf(client, data);
        break;

    case CLIENT_CMD_RECV_KEEP_ALIVE:
        this->send_keep_alive(client);
        break;

    default:
        lcd16x2_async_queue_push(this->lcd,
            "Invalid command!",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);
        break;

    }
}

/******************************************************************************/

static void
client_message_make_header(uint8_t cmd,
        void *data,
        size_t payload_len)
{
    client_message_t *header = data;

    header->cmd = cmd;
    header->id = HTONS(0x0123);
    header->n_bytes = HTONS(CLIENT_HEADER_LEN + payload_len);
    header->time_event = 0;
    header->time_send = 0;
    header->version = 0;
}

/******************************************************************************/

static size_t
client_message_make_led_state(client_real_t *this,
        void *data)
{
    uint8_t *payload = data;

    payload = this->message_pack_16(payload, (FIO2PIN0 & (1 << 3)) >> 3);

    return (size_t)(payload - (uint8_t *) data);
}

/******************************************************************************/

static uint8_t *
client_message_pack_16(uint8_t *pack,
        uint32_t value)
{
    size_t len = 0;

    len = sizeof (uint16_t);
    value = HTONS(value);

    memcpy(pack, (uint8_t *) & value, len);
    
    return pack + len;
}

/******************************************************************************/

static uint16_t
client_message_unpack_16(uint8_t *pack)
{
    uint16_t data = 0;

    memcpy((uint8_t *) &data, pack, 2);

    data = HTONS(data);

    return data;
}

/******************************************************************************/

client_t *
client_new(lcd16x2_t *lcd)
{
    client_real_t *obj = NULL;

    obj = calloc(1, sizeof (client_real_t));

    obj->lcd = lcd;

    obj->connect = client_connect;

    obj->closed = client_uip_closed;
    obj->connected = client_uip_connected;
    obj->led_off = client_led_off;
    obj->led_on = client_led_on;
    obj->message_get_cmd = client_message_get_cmd;
    obj->message_get_n_bytes = client_message_get_n_bytes;
    obj->message_handler = client_message_handler;
    obj->message_make_header = client_message_make_header;
    obj->message_make_led_state = client_message_make_led_state;
    obj->message_pack_16 = client_message_pack_16;
    obj->message_unpack_16 = client_message_unpack_16;
    obj->newdata = client_uip_newdata;
    obj->recv_dtmf = client_recv_dtmf;
    obj->send_keep_alive = client_send_keep_alive;
    obj->send_led_state = client_send_led_state;

    uip_ipaddr(obj->addr_serv, 192, 168, 0, 1);
    uip_ipaddr(obj->addr_local, 192, 168, 0, 2);
    uip_sethostaddr(obj->addr_local);

    return (client_t *) obj;
}

/******************************************************************************/

static void
client_recv_dtmf(client_t *client,
        client_message_t *message)
{
    uint16_t dtmf;
    uint8_t lcd_message[CLIENT_BUFFER_LEN] = {};
    client_real_t *this = (client_real_t *) client;

    dtmf = this->message_unpack_16(message->payload);

    snprintf(lcd_message, CLIENT_BUFFER_LEN, "DTMF=%02d", dtmf);

    switch (dtmf) {
    case 0:
        this->led_off();
        break;
        
    case 1:
        this->led_on();
        break;

    case 2:
        this->send_led_state(this);
        break;
    }

    lcd16x2_async_queue_push(this->lcd,
            lcd_message,
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);
}

/******************************************************************************/

static void
client_send_keep_alive(client_t *client)
{
    client_real_t *this = (client_real_t *) client;
    uint8_t buffer[CLIENT_BUFFER_LEN] = {};

    this->message_make_header(CLIENT_CMD_SEND_KEEP_ALIVE, buffer, 0);

    uip_send(buffer, this->message_get_n_bytes(buffer));
}

/******************************************************************************/

static void
client_send_led_state(client_real_t *this)
{
    client_message_t message = {};
    size_t payload_len = 0;

    payload_len = this->message_make_led_state(this, message.payload);

    this->message_make_header(CLIENT_CMD_SEND_LED_STATE, &message, payload_len);

    uip_send(&message, this->message_get_n_bytes(&message));
}

/******************************************************************************/

static void
client_uip_closed(client_t *client)
{
    client_real_t *this = (client_real_t *) client;

    lcd16x2_async_queue_push(this->lcd,
            "Closed!",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);

    vTaskDelay(5 * configTICK_RATE_HZ);

    this->connect(client);
}

/******************************************************************************/

static void
client_uip_connected(client_t *client)
{
    client_real_t *this = (client_real_t *) client;
    
    lcd16x2_async_queue_push(this->lcd,
            "Connected!",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);
}

/******************************************************************************/

static void
client_uip_newdata(client_t *client)
{
    client_real_t *this = (client_real_t *) client;

    if (uip_datalen() > 0) {
        this->message_handler(client, uip_appdata);
    }
}

/******************************************************************************/
