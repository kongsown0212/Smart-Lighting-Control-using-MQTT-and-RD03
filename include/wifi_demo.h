#ifndef WIFI_DEMO_H
#define WIFI_DEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <wifi_mgmr_ext.h>
#include <hal_wifi.h>
#include <aos/yloop.h>
#include <aos/kernel.h>
#include <lwip/tcpip.h>
#include "config_store.h"
#include "web_config.h"
#include "blog.h"



#define ROUTER_SSID "Nami R&D"
#define ROUTER_PWD "nami@2025"

void start_softap_mode(void);
void wifi_sta_connect(char* ssid, char* password);
extern void event_cb_wifi_event(input_event_t* event, void* private_data);
extern void proc_main_entry(void* pvParameters);
#endif // WIFI_DEMO_H
