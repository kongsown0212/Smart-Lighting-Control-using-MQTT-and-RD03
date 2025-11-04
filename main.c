#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include "blog.h"
#include <aos/yloop.h>
#include <aos/kernel.h>
#include <lwip/tcpip.h>
#include <wifi_mgmr_ext.h>
#include <hal_wifi.h>
#include "mqtt_demo.h"
#include "wifi_demo.h"

void main()
{   
    blog_set_level_log_component(BLOG_LEVEL_WARN, "tcp");      
    blog_set_level_log_component(BLOG_LEVEL_WARN, "axk_mqtt");
    puts("[OS] Starting TCP/IP Stack...");
    tcpip_init(NULL, NULL);
    puts("[OS] proc_main_entry task...");
    xTaskCreate(proc_main_entry, (char*)"main_entry", 1024, NULL, 15, NULL);
    
    xTaskCreate(TaskUart,(char*)"Task_Uart",1024,NULL,15,NULL);
}
    