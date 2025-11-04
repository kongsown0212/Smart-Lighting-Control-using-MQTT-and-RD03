#ifndef __MQTT_DEMO_H__
#define __MQTT_DEMO_H__

#include "mqtt_client.h"

static void log_error_if_nonzero(const char *message, int error_code);
static axk_err_t event_cb(axk_mqtt_event_handle_t event);
void mqtt_start(void);
void TaskUart(void *param);

extern axk_mqtt_client_handle_t client;

#endif