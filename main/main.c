#include <stdio.h>

#include "storage.h"
#include "wifi-manager.h"
#include "http-server.h"

#define DEFAULT_SSID        "ESP32-sensor"
#define DEFAULT_PASSWORD    "esp32sensor"


void wifi_connected(int e){
    if(e == 0){ // wifi connected
        printf("Wifi connected OK\n");
    }
    else{       // wifi not connected, run AP
        wifi_start_ap(DEFAULT_SSID, DEFAULT_PASSWORD);
    }

    http_server_start();
}

void wifi_try_connect(){
    esp_err_t rst;
    char ssid[32], passwd[32];

    rst = storage_read("ssid", ssid, sizeof(ssid));
    rst = storage_read("passwd", passwd, sizeof(passwd));

    if(rst == ESP_OK){   // wifi password set
        printf("Connect wifi to %s %s\n", ssid, passwd);
        wifi_connect(ssid, passwd, &wifi_connected);
    }
    else{       // wifi record not set, start AP
        printf("Wifi record not set, start AP");
        wifi_start_ap(DEFAULT_SSID, DEFAULT_PASSWORD);
    }
}

int app_main(void)
{
    storage_init();
    wifi_try_connect();

    return 0;
}