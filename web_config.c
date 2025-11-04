#include <stdio.h>
#include <string.h>
#include "lwip/api.h"
#include "easyflash.h"
#include "bl_sys.h"

#define SERVER_PORT 80

const static char http_form[] =
"HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><title>WB2 Config</title></head><body>"
"<h2>Enter WiFi & MQTT Info</h2>"
"<form action='/save' method='POST'>"
"SSID: <input name='ssid'><br>"
"Password: <input name='wifipass' type='password'><br>"
"MQTT Server: <input name='mqtt'><br>"
"MQTT User: <input name='mqtt_user'><br>"
"MQTT Pass: <input name='mqtt_pass' type='password'><br>"
"<input type='submit' value='Save & Connect'>"
"</form></body></html>";

static void parse_and_save_config(const char *body) {
    char ssid[64] = {0}, pass[64] = {0}, mqtt[64] = {0}, user[64] = {0}, mpass[64] = {0};

    
    sscanf(body, "ssid=%63[^&]&wifipass=%63[^&]&mqtt=%63[^&]&mqtt_user=%63[^&]&mqtt_pass=%63s",
           ssid, pass, mqtt, user, mpass);

    printf("Parsed SSID=%s, PASS=%s, MQTT=%s\n", ssid, pass, mqtt);

    // Lưu EasyFlash
    ef_set_env("wifi_ssid", ssid);
    ef_set_env("wifi_pass", pass);
    ef_set_env("mqtt_server", mqtt);
    ef_set_env("mqtt_user", user);
    ef_set_env("mqtt_pass", mpass);
    ef_save_env();

    vTaskDelay(pdMS_TO_TICKS(2000));
    bl_sys_reset_system();
}

static void web_http_server(struct netconn *conn) {
    struct netbuf *inputbuf;
    char *buf;
    u16_t buflen;

    if (netconn_recv(conn, &inputbuf) == ERR_OK) {
        netbuf_data(inputbuf, (void **)&buf, &buflen);
        buf[buflen] = 0; // null terminate

        printf("HTTP REQ:\n%s\n", buf);

        if (strncmp(buf, "GET / ", 6) == 0) {
            netconn_write(conn, http_form, strlen(http_form), NETCONN_NOCOPY);
        } 
        else if (strncmp(buf, "POST /save", 10) == 0) {
            char *body = strstr(buf, "\r\n\r\n");
            if (body) {
                body += 4; // skip header
                parse_and_save_config(body);
                char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nSaved! Rebooting...";
                netconn_write(conn, resp, strlen(resp), NETCONN_NOCOPY);
            }
        }
    }
    netconn_close(conn);
    netbuf_delete(inputbuf);
}

void http_server_start(void *pvParameters) {
    struct netconn *conn, *newconn;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, SERVER_PORT);
    netconn_listen(conn);

    while (1) {
        if (netconn_accept(conn, &newconn) == ERR_OK) {
            web_http_server(newconn);
            netconn_delete(newconn);
        }
    }
}
