#ifndef STORAGE_H
#define STORAGE_H

#include <nvs.h>
#include <esp_err.h>

nvs_handle_t storage_init();
void storage_close(nvs_handle_t *nvs);
esp_err_t storage_read(const char *key, char *value, size_t length);
esp_err_t storage_write(const char* key, const char* value);



#endif

