#include "http-server.h"

#include <esp_http_server.h>
#include <esp_log.h>

#include "storage.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))


static httpd_handle_t server = NULL;
static httpd_req_t *http_req;

static const char *TAG = "HTTP_SERVER";


/* An HTTP GET handler */
static esp_err_t connect_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    http_req = req;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    char ssid[32];
    char passwd[32];

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => ssid=%s", ssid);
            }
            if (httpd_query_key_value(buf, "password", passwd, sizeof(passwd)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => password=%s", passwd);
            }
        }
        free(buf);
    }

    storage_write("ssid", ssid);
    storage_write("passwd", passwd);

    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    esp_restart();

    return ESP_OK;
}

static const httpd_uri_t connect = {
    .uri       = "/connect",
    .method    = HTTP_GET,
    .handler   = connect_get_handler,
    .user_ctx  = "Connecting ...."
};

static esp_err_t default_get_handler(httpd_req_t *req){
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t defaultPage = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = default_get_handler,
    .user_ctx  = "<!DOCTYPE html><html><head><title>ESP32 temperature sensor</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /><style></style></head><body><h1>Connect to Wifi</h1><form action=\"/connect\"><h2>SSID</h2><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><br><h2>Password</h2><input type=\"text\" id=\"password\" name=\"password\"><br><br><input type=\"submit\" value=\"Connect\"></form></body></html>",
};



void http_server_start()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &defaultPage);
        httpd_register_uri_handler(server, &connect);
    }

    ESP_LOGI(TAG, "Error starting server!");
}

void http_server_stop()
{
    httpd_stop(server);
}
