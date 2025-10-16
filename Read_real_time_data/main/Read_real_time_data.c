#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "gateway.h"


#define WIFI_SSID     "MAUSO Office"
#define WIFI_PASS     "Mauso656768ms"
#define BACKEND_URL   "http://192.168.0.211:3000/api/data/Gateway1"
#define TAG           " DATA_LOGGER"

static void initialize_wifi(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi");
    ESP_ERROR_CHECK(nvs_flash_init());  //Initialize NVS
    ESP_ERROR_CHECK(esp_netif_init());  // network interface
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();  //Create default satation interface
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // Set station mode
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < 40) {
        ESP_LOGI(TAG, "Waiting for NTP time sync (%d/40)", retry + 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        retry++;
    }
    ESP_LOGI(TAG, "Time synchronized");
}
static void send_data_to_backend(const char *json_str) {
    esp_http_client_config_t config = {
        .url = BACKEND_URL,
        .method = HTTP_METHOD_POST,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST successful, status = %d",
                 esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

static void get_cambodia_time(char *buffer, size_t len) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    timeinfo.tm_hour += 7;
    mktime(&timeinfo); 
    strftime(buffer, len, "%Y-%m-%d_%H:%M:%S", &timeinfo);
}
void data_logger_task(void *pvParameter) {
    gateway_t gateway;
    char timestamp[32];

    while (1) {
       
        if (gateway_read_all(&gateway) == 0) {
           
            get_cambodia_time(timestamp, sizeof(timestamp));

    
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "gateway", "Gateway1");
            cJSON_AddStringToObject(root, "timestamp", timestamp);

       
            cJSON *adl = cJSON_CreateObject();
            cJSON_AddNumberToObject(adl, "voltage", gateway.adl400.voltage);
            cJSON_AddNumberToObject(adl, "current", gateway.adl400.current);
            cJSON_AddNumberToObject(adl, "active_power", gateway.adl400.active_power);
            cJSON_AddNumberToObject(adl, "reactive_power", gateway.adl400.reactive_power);
            cJSON_AddNumberToObject(adl, "power_factor", gateway.adl400.power_factor);
            cJSON_AddNumberToObject(adl, "frequency", gateway.adl400.frequency);
            cJSON_AddNumberToObject(adl, "total_energy", gateway.adl400.total_energy);
            cJSON_AddItemToObject(root, "ADL400", adl);

  
            cJSON *dtsd = cJSON_CreateObject();
            cJSON_AddNumberToObject(dtsd, "voltage", gateway.dtsd1352.voltage);
            cJSON_AddNumberToObject(dtsd, "current", gateway.dtsd1352.current);
            cJSON_AddNumberToObject(dtsd, "active_power", gateway.dtsd1352.active_power);
            cJSON_AddNumberToObject(dtsd, "reactive_power", gateway.dtsd1352.reactive_power);
            cJSON_AddNumberToObject(dtsd, "power_factor", gateway.dtsd1352.power_factor);
            cJSON_AddNumberToObject(dtsd, "frequency", gateway.dtsd1352.frequency);
            cJSON_AddNumberToObject(dtsd, "total_energy", gateway.dtsd1352.total_energy);
            cJSON_AddItemToObject(root, "DTSD1352", dtsd);

            char *json_str = cJSON_PrintUnformatted(root);
            send_data_to_backend(json_str);

            cJSON_Delete(root);
            free(json_str);
        } else {
            ESP_LOGW(TAG, "Failed to read meters");
        }

        vTaskDelay(60000 / portTICK_PERIOD_MS); 
    }
}

void app_main(void) {
    initialize_wifi();
    initialize_sntp();

    gateway_init();
    xTaskCreate(&data_logger_task, "data_logger_task", 8192, NULL, 5, NULL);
}



