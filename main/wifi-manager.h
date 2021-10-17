#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#define ESP_WIFI_MAXIMUM_RETRY 10

#define ESP_AP_CHANNEL  6
#define ESP_AP_MAX_CONN 10

void wifi_init();
void wifi_start_ap(const char* ssid, const char* passwd);
void wifi_connect(const char* ssid, const char* passwd, void (*connected)(int));

#endif