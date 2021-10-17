#include "http-server.h"
#include <esp_log.h>

#include "esp_system.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "string.h"

#include "storage.h"
#include "wifi-manager.h"

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const uint8_t indexHtmlStart[] asm("_binary_index_html_start"); //uint8_t
const uint8_t indexHtmlEnd[] asm("_binary_index_html_end");     //uint8_t

#define WIFI_NOTCONNECTED 0
#define WIFI_CONNECTED 1
#define WIFI_CONNECTING 2
#define WIFI_ERRORCONN 3

static uint8_t wifi_connected = WIFI_NOTCONNECTED;
static char ssid[32];

void http_wifi_callback(int e)
{
    wifi_connected = (e == 0) ? WIFI_ERRORCONN : WIFI_ERRORCONN;
}

static void http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;

    /* Read the data from the port, blocking if nothing yet there.
    We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);

    if (err == ERR_OK)
    {
        netbuf_data(inbuf, (void **)&buf, &buflen);

        if ((buflen >= 5) && (strncmp(buf, "GET /", 5) == 0))
        { // GET request
            printf("Request %s\n", buf);

            // buffer = GET /connect?ssid=Test&password=Aaaa HTTP/1.1
            if (strncmp(buf + 4, "/connect?", 8) == 0)
            { // API connect
                char passwd[32];

                char *bufssid = strstr(buf, "ssid") + 5;
                char *bufpass = strstr(buf, "password") + 9;

                if (bufssid[0] != '&' && bufpass[0] != ' ')
                { // not empty ssid and password
                    strncpy(ssid, bufssid, 31);
                    strncpy(passwd, bufpass, 31);

                    char *del = strstr(ssid, "&");
                    memset(del, 0, strlen(ssid) - strlen(del));

                    del = strstr(passwd, " ");
                    memset(del, 0, strlen(passwd) - strlen(del));

                    printf("Connect SSID: %s PASS: %s\n", ssid, passwd);
                    wifi_connected = WIFI_CONNECTING;

                    storage_write("ssid", ssid);
                    storage_write("passwd", passwd);

                    wifi_connect(ssid, passwd, &http_wifi_callback);
                }
            }
        }

        char connectingPage[64];
        netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);

        switch (wifi_connected)
        {
        case WIFI_NOTCONNECTED:
            netconn_write(conn, indexHtmlStart, indexHtmlEnd - indexHtmlStart - 1, NETCONN_NOCOPY);
            break;
        case WIFI_CONNECTED:
            snprintf(connectingPage, 64, "<h1>Connected to Wifi %s</h1>", ssid);
            netconn_write(conn, connectingPage, strlen(connectingPage), NETCONN_NOCOPY);
            break;
        case WIFI_CONNECTING:
            snprintf(connectingPage, 64, "<h1>Connecting to Wifi %s</h1>", ssid);
            netconn_write(conn, connectingPage, strlen(connectingPage), NETCONN_NOCOPY);
            break;
        case WIFI_ERRORCONN:
            snprintf(connectingPage, 64, "<h1>Error connect to Wifi %s</h1>", ssid);
            netconn_write(conn, connectingPage, strlen(connectingPage), NETCONN_NOCOPY);
            break;
        default:
            netconn_write(conn, indexHtmlStart, indexHtmlEnd - indexHtmlStart - 1, NETCONN_NOCOPY);
        }
    }

    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);
    netbuf_delete(inbuf);
}

static void http_server(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);

    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    do
    {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while (err == ERR_OK);

    netconn_close(conn);
    netconn_delete(conn);
}

void http_server_start()
{
    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
}

void http_server_stop()
{
}

void http_wifi_connected(char *ssid_connected)
{
    strcpy(ssid, ssid_connected);
    wifi_connected = WIFI_CONNECTED;
}
