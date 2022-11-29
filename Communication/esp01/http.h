#ifndef LIB_WIFI_HTTP_H_
#define LIB_WIFI_HTTP_H_

#include <csdriver.h>
#include <utils.h>
#include <esp8266.h>
#include "driverlib.h"

void http_init(void);
void http_request(void);

#endif /* LIB_WIFI_HTTP_H_ */
