#include <http.h>
#include <esp8266.h>
#include <utils.h>

static void connect_to_AP(void);
static void connect_to_server(void);
static void obtain_available_APs(void);
static void establish_uart_connection(void);
static void enable_multiple_connections(void);
static void query_server(const char *, uint32_t);
static void send_request(const char *, uint32_t);

static const char STOP_CAR[] = "GET /api/car/stop HTTP/1.1\r\n\r\n";
static const char START_CAR[] = "GET /api/start HTTP/1.1\r\n\r\n";
static const char GET_COMMAND[] = "GET /api/getCommands HTTP/1.1\r\n\r\n";
static char sendcommandtest[] = "GET /api/test/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/ HTTP/1.1\r\n\r\n";



static const char SSID[] = "Pogo";
static const char PASSWORD[] = "23456789";

static const char Port[] = "105";
static const char HTTP_WEBSERVER[] = "172.20.10.5";

static char *g_esp8266_data; // Pointer to ESP8266 global buffer

/**
 * Initialize the ESP8266 module
 */
void http_init(void)
{
    msprintf(EUSCI_A0_BASE, "Initialising ESP8266...\r\n\r\n");

    GPIO_Output(GPIO_PORT_P6, GPIO_PIN1);

    g_esp8266_data = esp8266_get_buffer();

    // Hard Reset ESP8266
    esp8266_hard_reset();
    delay(2000000);

    // Flush reset data, we do this because a lot of data received cannot be printed
    uart_flush(EUSCI_A2_BASE);

    // Establish connections
    establish_uart_connection();
    connect_to_AP();
    enable_multiple_connections();

    // Indicate that ESP8266 is ready
    LED2_GREEN_ON();
    delay(5000000);
    LED2_GREEN_OFF();
}

/**
 * Send HTTP request to the server and
 * move the car if there is a "command" keyword
 * in the response.
 *
 * Will request for command if car is not moving, else
 * call the api request to update the car state in db.
 */
void http_request(void)
{
    char command;

    if (1)
//    {
//        send_request(STOP_CAR, sizeof(STOP_CAR) - 1);
//    }
//    else
    {
        send_request(GET_COMMAND, sizeof(GET_COMMAND) - 1);
//        if (NULL != strstr(g_esp8266_data, "Command"))
//           {
          command = g_esp8266_data[strlen(g_esp8266_data) - 1];
          msprintf(EUSCI_A0_BASE, "Command: %c\r\n\r\n", command);
//           }
        //send_request(START_CAR, sizeof(START_CAR) - 1);

        // Check if there are commands to execute
//        if (NULL != strstr(g_esp8266_data, "Command"))
//        {
//            command = g_esp8266_data[strlen(g_esp8266_data) - 1];
//            msprintf(EUSCI_A0_BASE, "Command: %c\r\n\r\n", command);
////            l298n_move(command);
//
//            send_request(START_CAR, sizeof(START_CAR) - 1);
//        }
//        else if (NULL != strstr(g_esp8266_data, "None"))
//            msprintf(EUSCI_A0_BASE, "No commands at the moment.\r\n");
//        else
//            msprintf(EUSCI_A0_BASE, "Request successful\r\n");
    }
    msprintf(EUSCI_A0_BASE, "======================================\r\n");
}

/**
 * Establish UART connection with MSP432
 */
static void establish_uart_connection(void)
{
    if (!esp8266_check_connection())
    {
        msprintf(EUSCI_A0_BASE, "Failed MSP432 UART connection\r\n");
        msprintf(EUSCI_A0_BASE, "Attempting to reconnect UART...\r\n");

        // Retry if failed connection
        while (1)
        {
            if (esp8266_check_connection())
                break;
        }
    }

    msprintf(EUSCI_A0_BASE, "Nice! We are connected to the MSP432\r\n\r\n");
}

/**
 * Check available Access points
 */
static void obtain_available_APs(void)
{
    if (!esp8266_available_APs())
    {
        msprintf(EUSCI_A0_BASE, "Failed to obtain Access Points\n\r ERROR: %s \r\n", g_esp8266_data);
        msprintf(EUSCI_A0_BASE, "Attempting to obtain access point...\n\r");

        // Retry if failed to obtain access points
        while (1)
        {
            if (esp8266_available_APs())
            {
                msprintf(EUSCI_A0_BASE, "Successfully obtained Access Points\r\n\r\n");
                break;
            }
        }
    }

    msprintf(EUSCI_A0_BASE, "Got it! Here are the available Access Points: %s\r\n\r\n", g_esp8266_data);
}

/**
 * Connect to an Access Point
 */
static void connect_to_AP(void)
{
    msprintf(EUSCI_A0_BASE, "Attempting to connect to %s\r\n\r\n", SSID);

    if (!esp8266_connect_to_ap(SSID, PASSWORD))
    {
        msprintf(EUSCI_A0_BASE, "Failed to connect to %s\n\r", SSID);
        msprintf(EUSCI_A0_BASE, "Attempting to reconnect to %s\n\r", SSID);

        // Retry connection if we are not connected
        while (1)
        {
            if (esp8266_connect_to_ap(SSID, PASSWORD))
                break;
        }
    }

    msprintf(EUSCI_A0_BASE, "Connected to %s\n\r\n\r", SSID);
}

/**
 * Enable multiple connections, up to 4 according to the internet
 */
static void enable_multiple_connections(void)
{
    if (!esp8266_enable_multiple_connections(true))
    {
        msprintf(EUSCI_A0_BASE, "Failed to set multiple connections\r\n");

        // Retry if failed to set multiple connections
        while (1)
        {
            if (esp8266_enable_multiple_connections(true))
                break;
        }
    }

    msprintf(EUSCI_A0_BASE, "Multiple connections enabled\r\n");
    msprintf(EUSCI_A0_BASE, "===============================\r\n");
    delay(100000);
}

/**
 * Try to establish TCP connection to a HTTP server
 */
static void connect_to_server(void)
{
    if (!esp8266_establish_connection('0', TCP, HTTP_WEBSERVER, Port))
    {
        msprintf(EUSCI_A0_BASE, " this is a : %s\r\n", "12345678");
        msprintf(EUSCI_A0_BASE, "Failed to connect to: %s\r\n", HTTP_WEBSERVER);
        msprintf(EUSCI_A0_BASE, "Attempting to reconnect to: %s\r\n", HTTP_WEBSERVER);

        // Retry if failed to connect to server
        while (1)
        {
            if (esp8266_establish_connection('0', TCP, HTTP_WEBSERVER, Port))
                break;
        };
    }

    msprintf(EUSCI_A0_BASE, "Connected to: %s\r\n\r\n", HTTP_WEBSERVER);
}

/**
 * Query data to connected HTTP server
 */
static void query_server(const char *request, uint32_t request_size)
{
    if (!esp8266_send_data('0', request, request_size))
    {
        msprintf(EUSCI_A0_BASE, "Failed to send: %s\r\n", request);
        msprintf(EUSCI_A0_BASE, "Attempting to reconnect to: %s\r\n", HTTP_WEBSERVER);

        // Retry if failed to send data
        while (1)
        {
            connect_to_server();
            if (esp8266_send_data('0', request, request_size))
                break;
        };
    }else{
        msprintf(EUSCI_A0_BASE, "Connected");
    }
}

/**
 * Connect to server and send request.
 *
 * Turn on BLUE LED to indicate that ESP8266 is sending request to the server.
 */
static void send_request(const char *request_url, uint32_t request_size)
{
    LED2_BLUE_ON();

    connect_to_server();
    query_server(request_url, request_size);

    LED2_BLUE_OFF();
    delay(1000000);
}
