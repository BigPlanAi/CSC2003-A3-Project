#include <esp8266.h>

#define RESET_PIN GPIO_PIN1
#define RESET_PORT GPIO_PORT_P6
#define AT "AT\r\n"
#define AT_RST "AT+RST\r\n"
#define AT_GMR "AT+GMR\r\n"
#define AT_GSLP "AT+GSLP\r\n"
#define ATE "ATE\r\n"
#define AT_CWMODE "AT+CWMODE=3\r\n"
#define AT_CWJAP "AT+CWJAP"
#define AT_CWLAP "AT+CWLAP\r\n"
#define AT_CWQAP "AT+CWQAP\r\n"
#define AT_CWLIF "AT+CWLIF\r\n"
#define AT_CWDHCP "AT+CWDHCP\r\n"
#define AT_CIPSTAMAC "AT+CIPSTAMAC\r\n"
#define AT_CIPAPMAC "AT+CIPAPMAC\r\n"
#define AT_CIPSTA "AT+CIPSTA\r\n"
#define AT_CIPAP "AT+CIPAP"
#define AT_CIPSTATUS "AT+CIPSTATUS\r\n"
#define AT_CIPSTART "AT+CIPSTART"
#define AT_CIPSEND "AT+CIPSEND"
#define AT_CIPCLOSE "AT+CIPSEND\r\n"
#define AT_CIFSR "AT+CIFSR\r\n"
#define AT_CIPMUX "AT+CIPMUX"
#define AT_CIPSERVER "AT+CIPSTO\r\n"
#define AT_CIPMODE "AT+CIPMODE\r\n"
#define AT_CIPSTO "AT+CIPSTO\r\n"
#define AT_CIUPDATE "AT+CIUPDATE\r\n"
#define IPD "+IPD\r\n"

char esp8266_buffer[ESP8266_BUFFER_SIZE];
//static const char mode[] = "2";

/**
 * Wait for a response from the ESP8266.
 */
bool esp8266_wait_for_answer(uint32_t tries)
{
    uint32_t c;
    uint32_t i = 0;

    while (tries)
    {
        if (uart_available(EUSCI_A2_BASE))
        {
            while (uart_available(EUSCI_A2_BASE))
            {
                uart_read(EUSCI_A2_BASE, (uint8_t *)&c, 1);

                if (i > ESP8266_BUFFER_SIZE)
                {
                    return false;
                }
                else
                {
                    esp8266_buffer[i++] = c;
                }
            }

            esp8266_buffer[i++] = 0;
            return true;
        }
        tries--;
        __delay_cycles(2400);
    }

    return false;
}

/**
 * Check if the ESP8266 is ready to receive commands.
 */
bool esp8266_check_connection(void)
{

    msprintf(EUSCI_A2_BASE, AT);
    __delay_cycles(12000);
    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Retrieve all available access points.
 */
bool esp8266_available_APs(void)
{
//    msprintf(EUSCI_A2_BASE, "%s=\"%s\"\r\n", AT_CWMODE, mode);
    msprintf(EUSCI_A2_BASE, AT_CWLAP);
    __delay_cycles(48000000);

    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Connect to an access point
 */
bool esp8266_connect_to_ap(const char *ssid, const char *password)
{
    msprintf(EUSCI_A2_BASE, "%s=\"%s\",\"%s\"\r\n", AT_CWJAP, ssid, password);
    __delay_cycles(40000000);

    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Enable multiple connections
 */
bool esp8266_enable_multiple_connections(bool enable)
{
    char c;

    switch (enable)
    {
    case 0:
        c = '0';
        break;
    case 1:
        c = '1';
        break;
    }

    msprintf(EUSCI_A2_BASE, "%s=%c\r\n", AT_CIPMUX, c);

    __delay_cycles(5000000);
    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Establish connection to the server
 */
bool esp8266_establish_connection(const char id, const uint8_t type, const char *address, const char *port)
{
    char ct[3];

    switch (type)
    {
    case TCP:
        ct[0] = 'T';
        ct[1] = 'C';
        ct[2] = 'P';
        break;
    case UDP:
        ct[0] = 'U';
        ct[1] = 'D';
        ct[2] = 'P';
        break;
    }

    msprintf(EUSCI_A2_BASE, "%s=%c,\"TCP\",\"%s\",%s\r\n", AT_CIPSTART, id, address, port);
    __delay_cycles(20000000);

    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL != strstr(esp8266_buffer, "ALREADY CONNECTED"))
    {
        return true;
    }

    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Send request to the server.
 */
bool esp8266_send_data(const char id, const char *data, const uint32_t data_size)
{
    char size[5];

    ltoa(data_size, size);
    msprintf(EUSCI_A2_BASE, "%s=%c,%s\r\n", AT_CIPSEND, id, size); //Multiple connections:(+ CIPMUX = 1) AT + CIPSEND = <ID>, <length>

    __delay_cycles(8000000);
    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }

    if (NULL == strstr(esp8266_buffer, ">"))
    {
        return false;
    }

    msprintf(EUSCI_A2_BASE, data);
    msprintf(EUSCI_A0_BASE, "Request: %s", data);

    __delay_cycles(8000000);
    if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
    {
        return false;
    }
    if (NULL == strstr(esp8266_buffer, "OK"))
    {
        return false;
    }

    return true;
}

/**
 * Serial Terminal
 */
void esp8266_terminal(void)
{
    while (1)
    {
        msprintf(EUSCI_A2_BASE, esp8266_buffer);

        __delay_cycles(48000000);
        if (!esp8266_wait_for_answer(ESP8266_RECEIVE_TRIES))
        {
            msprintf(EUSCI_A0_BASE, "ESP8266 receive timeout error\r\n");
        }
        else
        {
            msprintf(EUSCI_A0_BASE, esp8266_buffer);
        }
    }
}

/**
 * Retrieve buffer of received data.
 */
char *esp8266_get_buffer(void)
{
    return esp8266_buffer;
}

/**
 * Resets the ESP8266.
 */
void esp8266_hard_reset(void)
{
    GPIO_Low(RESET_PORT, RESET_PIN);

    __delay_cycles(24000000);

    GPIO_High(RESET_PORT, RESET_PIN);
}
