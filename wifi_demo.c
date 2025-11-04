#include "wifi_demo.h"
#include "blog.h"
#include <wifi_mgmr_ext.h>

static wifi_conf_t conf =
{
    .country_code = "CN",
};

void start_softap_mode(void) {
    wifi_interface_t wifi_ap;
    wifi_ap = wifi_mgmr_ap_enable();
    wifi_mgmr_ap_start(wifi_ap, "WB2_Config_1234", 0, "12345678", 6); 
    printf("SoftAP started, connect to SSID: WB2_Config_1234\n");
}
void wifi_sta_connect(char* ssid, char* password)
{
    wifi_interface_t wifi_interface;

    wifi_interface = wifi_mgmr_sta_enable(); //bật Wi-Fi Station (thiết bị làm client).
    wifi_mgmr_sta_connect(wifi_interface, ssid, password, NULL, NULL, 0, 0); //kết nối vào AP với SSID/PWD.
}

extern void event_cb_wifi_event(input_event_t* event, void* private_data)
{
    switch (event->code)
    {
        case CODE_WIFI_ON_INIT_DONE: //Wi-Fi driver đã init xong
        {
            blog_info("[APP] [EVT] INIT DONE %lld", aos_now_ms());
            wifi_mgmr_start_background(&conf);
            blog_info("wifi_mgmr_start_background() called");
        }
        break;
        case CODE_WIFI_ON_MGMR_DONE: //Wi-Fi quản lý (manager) đã sẵn sàng
        {
            blog_info("[APP] [EVT] MGMR DONE %lld", aos_now_ms());
            char ssid[64], pass[64];
        if (config_get_wifi(ssid, pass, sizeof(ssid))) {
            
            wifi_sta_connect(ssid, pass);
        } else {
            start_softap_mode();
            http_server_start(); // khởi động web server
        }
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE:
        {
            blog_info("[APP] [EVT] SCAN Done %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_DISCONNECT:
        {
            blog_info("[APP] [EVT] disconnect %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_CONNECTING: 
        {
            blog_info("[APP] [EVT] Connecting %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_CMD_RECONNECT:
        {
            blog_info("[APP] [EVT] Reconnect %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_CONNECTED://đã kết nối thành công tới AP.
        {
            blog_info("[APP] [EVT] connected %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_PRE_GOT_IP:
        {
            blog_info("[APP] [EVT] connected %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_GOT_IP://đã nhận được IP từ DHCP.
        {
            blog_info("[APP] [EVT] GOT IP %lld", aos_now_ms());
            blog_info("[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());
            extern void mqtt_start(void);
            mqtt_start(); //bắt đầu demo MQTT
        }
        break;
        case CODE_WIFI_ON_PROV_SSID:
        break;
        case CODE_WIFI_ON_PROV_BSSID:
        {
            blog_info("[APP] [EVT] [PROV] [BSSID] %lld: %s",
                   aos_now_ms(),
                   event->value ? (const char*)event->value : "UNKNOWN");
            if (event->value)
            {
                vPortFree((void*)event->value);
            }
        }
        break;
        case CODE_WIFI_ON_PROV_PASSWD:
        break;
        case CODE_WIFI_ON_PROV_CONNECT:
        break;
        case CODE_WIFI_ON_PROV_DISCONNECT:
        {
            blog_info("[APP] [EVT] [PROV] [DISCONNECT] %lld", aos_now_ms());
        }
        break;
        default:
        {
            blog_info("[APP] [EVT] Unknown code %u, %lld", event->code, aos_now_ms());
            /*nothing*/
        }
    }
}




extern void proc_main_entry(void* pvParameters)
{


    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL); //đăng ký callback sự kiện Wi-Fi
    blog_info("Starting hal_wifi_start_firmware_task()");
    hal_wifi_start_firmware_task();  //khởi động task firmware Wi-Fi
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0); //gửi sự kiện Wi-Fi driver đã init xong
    blog_info("Posted CODE_WIFI_ON_INIT_DONE");
     vTaskDelete(NULL);
}

