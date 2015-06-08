#ifndef NETWORK_CMDS
#define NETWORK_CMDS

#include "c_types.h"
#include "espconn.h"

void ICACHE_FLASH_ATTR connect_to_network(const char *ssid, const char *password, struct espconn *conn);
void ICACHE_FLASH_ATTR udp_send_ipmac(struct espconn *conn);

#endif
