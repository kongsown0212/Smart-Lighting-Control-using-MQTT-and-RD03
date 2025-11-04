#include "config_store.h"
#include "easyflash.h"
#include <string.h>

// ---- WiFi ----
void config_set_wifi(const char *ssid, const char *pass) {
    ef_set_env("wifi_ssid", ssid);
    ef_set_env("wifi_pass", pass);
    ef_save_env();   // nhớ lưu xuống Flash
}

bool config_get_wifi(char *ssid, char *pass, int len) {
    char *s = ef_get_env("wifi_ssid");
    char *p = ef_get_env("wifi_pass");
    if (s && p) {
        strncpy(ssid, s, len-1);
        strncpy(pass, p, len-1);
        return true;
    }
    return false; // chưa có
}

// ---- MQTT ----
void config_set_mqtt(const char *server, const char *user, const char *pass) {
    ef_set_env("mqtt_server", server);
    ef_set_env("mqtt_user", user);
    ef_set_env("mqtt_pass", pass);
    ef_save_env();
}

bool config_get_mqtt(char *server, char *user, char *pass, int len) {
    char *s = ef_get_env("mqtt_server");
    char *u = ef_get_env("mqtt_user");
    char *p = ef_get_env("mqtt_pass");
    if (s && u && p) {
        strncpy(server, s, len-1);
        strncpy(user, u, len-1);
        strncpy(pass, p, len-1);
        return true;
    }
    return false;
}

// ---- Xóa hết ----
void config_clear_all(void) {
    ef_del_env("wifi_ssid");
    ef_del_env("wifi_pass");
    ef_del_env("mqtt_server");
    ef_del_env("mqtt_user");
    ef_del_env("mqtt_pass");
    ef_save_env();
}
