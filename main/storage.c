#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "storage.h"

static nvs_handle_t my_handle;

nvs_handle_t storage_init()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("STORAGE Ready\n");
    }

    return my_handle;
}

// destructor
void storage_close(nvs_handle_t *nvs)
{
    nvs_close(my_handle);
}

// Read
esp_err_t storage_read(const char *key, char *value, size_t length)
{
    esp_err_t err;
    
    err = nvs_get_str(my_handle, key, value, &length);
    switch (err)
    {
    case ESP_OK:
        printf("STORAGE Read %s = %s\n", key, value);
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("STORAGE The value is not initialized yet!\n");
        break;
    default:
        printf("STORAGE Error (%s) reading!\n", esp_err_to_name(err));
    }

    return err;
}

// Write
esp_err_t storage_write(const char *key, const char *value)
{
    esp_err_t err;
    err = nvs_set_str(my_handle, key, value);
    ESP_ERROR_CHECK(err);
    err = nvs_commit(my_handle);
    ESP_ERROR_CHECK(err);
    return err;
}
