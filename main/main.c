#include <stdio.h>

#include "storage.h"
#include "wifi-manager.h"
#include "http-server.h"
#include "dns-captive.h"

#define DEFAULT_SSID        "ESP32-sensor"
#define DEFAULT_PASSWORD    "esp32sensor"

char ssid[32], passwd[32];


void wifi_callback(int e){
    if(e == 0){ // wifi connected
        printf("Wifi connected OK\n");
    }
    else{       // wifi not connected, run AP
        printf("Connecting failed, starting AP\n");
        wifi_start_ap(DEFAULT_SSID, DEFAULT_PASSWORD);
    }

    http_server_start();
}

void wifi_try_connect(){
    esp_err_t rst;

    rst = storage_read("ssid", ssid, sizeof(ssid));
    rst = storage_read("passwd", passwd, sizeof(passwd));

    if(rst == ESP_OK){   // wifi password set
        printf("Connect wifi to %s with %s\n", ssid, passwd);
        wifi_connect(ssid, passwd, &wifi_callback);
    }
    else{       // wifi record not set, start AP
        printf("Wifi record not set, start AP");
        http_server_start();
    }
}

int app_main(void)
{
    storage_init();
    
    // wifi_init();

    wifi_try_connect();
    
    dns_captive_init();
    

    return 0;
}