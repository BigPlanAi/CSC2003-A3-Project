#ifndef LIB_WIFI_ESP8266_H_
#define LIB_WIFI_ESP8266_H_

#include <string.h>
#include <stdlib.h>
#include <utils.h>
#include <mspio.h>

#define ESP8266_BUFFER_SIZE 2048
#define ESP8266_RECEIVE_TRIES 100

#define TCP 0
#define UDP 1

extern char esp8266_buffer[ESP8266_BUFFER_SIZE];

bool esp8266_check_connection(void);
bool esp8266_available_APs(void);
bool esp8266_connect_to_ap(const char *, const char *);
bool esp8266_establish_connection(const char, const uint8_t, const char *, const char *);
bool esp8266_enable_multiple_connections(bool);
bool esp8266_send_data(const char, const char *, const uint32_t);

void esp8266_send_AT_command(char *);
void esp8266_terminal(void);
char *esp8266_get_buffer(void);
void esp8266_hard_reset(void);

#endif /* LIB_WIFI_ESP8266_H_ */
