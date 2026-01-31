#ifndef SARA_CHARM_SERVER_SERVER_H
#define SARA_CHARM_SERVER_SERVER_H


#include <esp_http_server.h>


typedef httpd_handle_t server_handle_t;

server_handle_t start_server(void);


#endif //!SARA_CHARM_SERVER_SERVER_H
