#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "esp_err.h"

#define UDP_PORT 3333

esp_err_t udp_server_start(void);
void udp_server_stop(void);

#endif // UDP_SERVER_H
