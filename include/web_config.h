#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H
#include "event_groups.h"
#include "lwip/err.h"
#include "string.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"


void parse_and_save_config(const char *body);
void web_http_server(struct netconn *conn);
void http_server_start(void);
#endif // WEB_CONFIG_H
