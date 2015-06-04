#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "user_config.h"

#include "sta_server.h"
#include "ap_server.h"
#include "helper.h"

bool HAS_BEEN_CONNECTED_AS_STATION;
bool HAS_RECEIVED_CONNECT_INSTRUCTION;

int TEMPERATURE;

void ICACHE_FLASH_ATTR init_done()
{
	const char *opmodes[] = {"NONE", "STATION_MODE", "SOFTAP_MODE", "STATIONAP_MODE"};
	struct softap_config config;
	struct ip_info info;
	uint8 opmode;
	enum dhcp_status status;

	opmode = wifi_get_opmode();

	if (opmode != SOFTAP_MODE)
		return;

	ets_uart_printf("Current Opmode: 0x%02x (%s)\n", opmode, (opmode < 4 ? opmodes[opmode] : "???"));

	ets_uart_printf("\n");

	if (!wifi_softap_get_config(&config))
		ets_uart_printf("Failed to get wifi softap config.\n");
	else
		print_softap_config(&config);

	ets_uart_printf("\n");

	if (!wifi_get_ip_info(SOFTAP_IF, &info))
		ets_uart_printf("Failed to get ip info.\n");
	else
		print_ip_info(&info);

	ets_uart_printf("\n");

	status = wifi_softap_dhcps_status();

	ets_uart_printf("DHCP server status:\t%d\n", status);
	ets_uart_printf("\n");

	if (ap_server_init() != 0)
		ets_uart_printf("Failed to initialize ap server.\n");
}

void ICACHE_FLASH_ATTR wifi_timer_cb(void *timer_arg)
{
	char ssid[32];
	char password[64] = DEFAULT_AP_PASSWORD;
	uint8 channel = DEFAULT_AP_CHANNEL;

	generate_default_ssid(ssid, sizeof ssid);

	if (!HAS_BEEN_CONNECTED_AS_STATION) {
		ets_uart_printf("Auto connect wifi timeout\n");
		wifi_set_opmode(SOFTAP_MODE);

		if (start_access_point(ssid, password, channel) != 0)
			ets_uart_printf("Failed to start access point.\n");

		init_done();		
	}
}

void ICACHE_FLASH_ATTR user_init()
{
	static ETSTimer wifi_connect_timer;

//	system_restore();
	system_set_os_print(0);
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	ets_uart_printf("\n\n\n");

	HAS_RECEIVED_CONNECT_INSTRUCTION = false;
	HAS_BEEN_CONNECTED_AS_STATION = false;

	if (is_flash_used()) {
		struct DeviceConfig conf;

		if (read_device_config(&conf) != 0) {
			ets_uart_printf("Failed to read device config.\n");
		} else {
			ets_uart_printf("Current device config: device_name = %s, device_type = %s\n",
					conf.device_name, conf.device_type);
		}
	} else {
		ets_uart_printf("This device has not yet been configured.\n");
	}

	wifi_set_opmode(STATION_MODE);
	wifi_set_event_handler_cb(sta_wifi_handler);

	if (!wifi_station_set_auto_connect(1))
		ets_uart_printf("Unable to set auto connect.\n");
	
	system_init_done_cb(&init_done);

	os_timer_setfn(&wifi_connect_timer, wifi_timer_cb, NULL);
	os_timer_arm(&wifi_connect_timer, AUTO_CONNECT_TIMEOUT, false); 
}
