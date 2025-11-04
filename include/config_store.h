#ifndef __CONFIG_STORE_H__
#define __CONFIG_STORE_H__

#include <stdbool.h>

// --- WiFi ---
void config_set_wifi(const char *ssid, const char *pass);
bool config_get_wifi(char *ssid, char *pass, int len);

// --- MQTT ---
void config_set_mqtt(const char *server, const char *user, const char *pass);
bool config_get_mqtt(char *server, char *user, char *pass, int len);

// --- Reset all config ---
void config_clear_all(void);

#endif
