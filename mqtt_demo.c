#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <mqtt_client.h>
#include "blog.h"
#include "string.h"
#include <hosal_uart.h>
#include "timers.h"
#include "bl_gpio.h"
#include <stdlib.h>
#include <bl_sys.h>
#include <math.h>
#include <mqtt_demo.h>

#define LED_GPIO 14  
#define KEEP_TIME_MS 5000
hosal_uart_dev_t uart_dev_0;
hosal_uart_dev_t uart_dev_1;
volatile bool uart_trigger = false;  
bool ack_ready=false; //Check for callback data
bool check_status=false; //Check for status rd03
int check_last_status = 0;
uint8_t tx_buf[31]; //Receive data from MQTTX
int tx_len = 0; 
uint8_t ack_buf[64]; //Buffer for Uart
uint8_t temp_buf[64];
int ret=0;
static uint8_t hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0xFF; 
}

int hex_string_to_bytes(const char *hex_str, uint8_t *out_buf, int max_len) {
    int len = 0;
    const char *p = hex_str;

    while (*p != '\0' && len < max_len) {
      
        while (*p == ' ' || *p == ',' || *p == '{' || *p == '}') p++;
        if (*p == '\0') break;

        char h1 = *p++;
        char h2 = *p++;

        uint8_t hi = hex_char_to_val(h1);
        uint8_t lo = hex_char_to_val(h2);
        if (hi == 0xFF || lo == 0xFF) continue; 

        out_buf[len++] = (hi << 4) | lo;
    }

    return len;
}



#define HEADER_LEN 4
#define END_LEN 4
#define START_BYTE_DATA 6 

typedef struct 
{
    uint8_t length;
    uint8_t header[HEADER_LEN];
    uint8_t end[END_LEN];
    uint8_t data[64];;
    uint8_t parameter_id;
    uint8_t parameter_value[3];
} rd03_t;


rd03_t rd03={
    .length=0,
    .parameter_id=0,
};
uint32_t last_detect_time = 0;
axk_mqtt_client_handle_t client;


void send_ack (rd03_t buffer){
    //open command
    if(buffer.data[0]==0xFF && buffer.data[1]==0x01 && buffer.data[4]==0x02 && buffer.data[6]==0x20){
        axk_mqtt_client_publish(client, "/topic/pre/ack/", "open sensor", 0, 0, 0);
    }

    //close command
    else if( buffer.data[0]==0xFE && buffer.data[1]==0x01 && buffer.data[2]==0x00 && buffer.data[3]==0x00){
        axk_mqtt_client_publish(client, "/topic/pre/ack/", "close sensor", 0, 0, 0);
    }

    //manual configuration command
    else if (buffer.data[0]==0x07 && buffer.data[1]==0x01 && buffer.data[2]==0x00 && buffer.data[3]==0x00)
    {
        axk_mqtt_client_publish(client, "/topic/pre/ack/", "config success", 0, 0, 0);
    }
    //read value command
    else if (buffer.data[0]==0x08 && buffer.data[1]==0x01 && buffer.data[2]==0x00 && buffer.data[3]==0x00)
    {
        float value;
        value = buffer.data[4] | ( buffer.data[5]<<0x08) | ( buffer.data[6]<<0x10) | ( buffer.data[7]<<0x18);
        value = 10*log10(value);
        char parameter_name[64]="";
        if(rd03.parameter_id>=0x10 && rd03.parameter_id<=0x1F){
            sprintf(parameter_name, "Trigger F%d:%.2f", rd03.parameter_id - 0x10,value);
        }
        else if(rd03.parameter_id>=0x20 && rd03.parameter_id<=0x2F){
            sprintf(parameter_name, "Motion hold F%d:%.2f", rd03.parameter_id - 0x20,value);
        }
        else if(rd03.parameter_id>=0x30 && rd03.parameter_id<=0x3F){
            sprintf(parameter_name, "Motion hold F%d:%.2f", rd03.parameter_id - 0x30,value);
        } 
        axk_mqtt_client_publish(client, "/topic/pre/ack/",parameter_name, 0, 0, 0);
        
    }
      
     //auto configuration command
    
    else if (buffer.data[0]==0x09 && buffer.data[1]==0x01 && buffer.data[2]==0x00 && buffer.data[3]==0x00)
    {
        uint8_t parameter_buffer[3]; 
        char command_ack[64]="";
        parameter_buffer[0]=rd03.parameter_value[0]/0x0A; //trigger
        parameter_buffer[1]=rd03.parameter_value[1]/0x0A; //hold
        parameter_buffer[2]=rd03.parameter_value[2]/0x0A; //micro

        sprintf(command_ack, "Trigger:%d  Hold:%d  Micro:%d", parameter_buffer[0],parameter_buffer[1],parameter_buffer[2]);
        axk_mqtt_client_publish(client,"/topic/pre/ack/",command_ack, 0, 0, 0);
    }

    //check auto load percentage
    else if (buffer.data[0]==0x0A && buffer.data[1]==0x01 && buffer.data[2]==0x00 && buffer.data[3]==0x00){
        int percent = buffer.data[4];
        char command_ack[64]="";
        if (percent==100){
        sprintf(command_ack, "ACK: Done, Percent:%d",percent); 
        }
        else sprintf(command_ack, "ACK: Loading, Percent:%d",percent); 
        axk_mqtt_client_publish(client,"/topic/pre/ack/",command_ack, 0, 0, 0);
    }

}

void rd03_handle_frame(){
    uint16_t count = 0;
    //frame header
    memcpy(&rd03.header[0],&temp_buf[count], HEADER_LEN);
    count += HEADER_LEN;
    //frame data length
    rd03.length=temp_buf[count];
    count +=2;
    //frame data
    memcpy(&rd03.data[0],&temp_buf[count],rd03.length);
    count+=rd03.length;
    //frame end 
    memcpy(&rd03.end[0],&temp_buf[count],END_LEN);
}

// --- TIMER CALLBACK ---
void vTimerCallback(TimerHandle_t xTimer)
{
    uart_trigger=true;
    
}

// --- TASK UART ---
void TaskUart(void *param)
{
    blog_info("UART + Timer Echo Task Start");

    bl_gpio_enable_output(LED_GPIO, 0, 0);

    uart_dev_0.config.uart_id = 0;
    uart_dev_0.config.tx_pin = 4;
    uart_dev_0.config.rx_pin = 3;
    uart_dev_0.config.cts_pin = 255;
    uart_dev_0.config.rts_pin = 255;
    uart_dev_0.config.baud_rate = 115200;
    uart_dev_0.config.data_width = HOSAL_DATA_WIDTH_8BIT;
    uart_dev_0.config.parity = HOSAL_NO_PARITY;
    uart_dev_0.config.stop_bits = HOSAL_STOP_BITS_1;
    uart_dev_0.config.mode = HOSAL_UART_MODE_POLL;

    uart_dev_1.config.uart_id = 1;
    uart_dev_1.config.tx_pin = 16;
    uart_dev_1.config.rx_pin = 7;
    uart_dev_1.config.cts_pin = 255;
    uart_dev_1.config.rts_pin = 255;
    uart_dev_1.config.baud_rate = 115200;
    uart_dev_1.config.data_width = HOSAL_DATA_WIDTH_8BIT;
    uart_dev_1.config.parity = HOSAL_NO_PARITY;
    uart_dev_1.config.stop_bits = HOSAL_STOP_BITS_1;
    uart_dev_1.config.mode = HOSAL_UART_MODE_POLL;

    hosal_uart_init(&uart_dev_0);
    hosal_uart_init(&uart_dev_1);

    vTaskDelay(pdMS_TO_TICKS(2000));

    TimerHandle_t uartTimer = xTimerCreate(
        "UartTimer",
        pdMS_TO_TICKS(10),  
        pdTRUE,
        (void *)0,
        vTimerCallback);

    if (uartTimer != NULL)
        xTimerStart(uartTimer, 0);

    while (1) {
        if(uart_trigger){
            uart_trigger=false;
            ret= hosal_uart_receive(&uart_dev_0,temp_buf,sizeof(temp_buf));
            if (ret > 0) {
                int start_index = -1;
                for (int i = 0; i < ret; i++) {
                    if (temp_buf[i] == 0xFD) {  
                        start_index = i;
                        break;
                    }
                }
                if (start_index >= 0 && (ret - start_index) > HEADER_LEN + 2 + END_LEN) {
                memmove(temp_buf, &temp_buf[start_index], ret - start_index);
                ret = ret - start_index;
                rd03_handle_frame();
                send_ack(rd03);
                }
                else {
                    hosal_uart_send(&uart_dev_1,temp_buf,ret);
                    check_status=true;
                }
            }
        }

        if(check_status){
            if(strncmp((char *)temp_buf, "distance:", 9) == 0 && check_last_status==0){
                axk_mqtt_client_publish(client,"/topic/pre/status/","status: Have Presence", 0, 0, 0);
                bl_gpio_output_set(LED_GPIO,1);
                check_last_status=1;
            }
            else if(strncmp((char *)temp_buf, "OFF", 3)==0 && check_last_status==1){
                axk_mqtt_client_publish(client,"/topic/pre/status/","status: OFF", 0, 0, 0);
                bl_gpio_output_set(LED_GPIO,0);
                check_last_status=0;
            }
        }

         vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}


static void log_error_if_nonzero(const char *message, int error_code) 
{
    if (error_code != 0) {
        blog_error("Last error %s: 0x%x", message, error_code);
    }
}

static axk_err_t event_cb(axk_mqtt_event_handle_t event)
{
    int32_t event_id;
    client = event->client;

    event_id = event->event_id;
    blog_debug("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((axk_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        blog_info("MQTT_EVENT_CONNECTED");
        msg_id = axk_mqtt_client_publish(client, "/topic/ack/qos1", "connected", 0, 1, 0);
        blog_info("sent publish successful, msg_id=%d", msg_id);

        msg_id = axk_mqtt_client_subscribe(client, "/topic/pre_test/qos0", 0);
        blog_info("sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        blog_info("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        blog_info("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        blog_info("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        blog_info("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        blog_info("MQTT_EVENT_DATA");
        tx_len= hex_string_to_bytes(event->data, tx_buf, sizeof(tx_buf));
        if(tx_buf[6]==0x07 || tx_buf[6]==0x08){
            rd03.parameter_id=tx_buf[8];
        }
        else if(tx_buf[6]==0x09){
            rd03.parameter_value[0]=tx_buf[8];
            rd03.parameter_value[1]=tx_buf[10];
            rd03.parameter_value[2]=tx_buf[12];
        }
        hosal_uart_send(&uart_dev_0, tx_buf, tx_len);
        break;
    case MQTT_EVENT_ERROR:
        blog_info("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from axk-tls", event->error_handle->axk_tls_last_axk_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->axk_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->axk_transport_sock_errno);
            blog_info("Last errno string (%s)", strerror(event->error_handle->axk_transport_sock_errno));
        }
        break;
    default:
        blog_info("Other event id:%d", event->event_id);
        break;
    }
    return AXK_OK;
}

void mqtt_start(void)
{
     

    axk_mqtt_client_config_t mqtt_cfg = {
        .uri = "your-url", // tcp
        .event_handle = event_cb,
        .lwt_retain = 1,
        .username = "your-username",
        .password = "your-password",
        .keepalive = 25
    };
    
    client = axk_mqtt_client_init(&mqtt_cfg);
    axk_mqtt_client_start(client);
}


