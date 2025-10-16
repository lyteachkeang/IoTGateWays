#include "firebase.h"
#include "firebase_ca.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "lwip/netdb.h"

static const char *TAG = "FIREBASE";

#define FIREBASE_HOST "realtime-database-d5c4c-default-rtdb.asia-southeast1.firebasedatabase.app"

void firebase_init(void) {
    ESP_LOGI(TAG, "Firebase initialized");
}

static int check_dns(void) {
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    int err = getaddrinfo(FIREBASE_HOST, "443", &hints, &res);
    if (err != 0) {
        ESP_LOGE(TAG, "DNS lookup failed for %s: %d", FIREBASE_HOST, err);
        return -1;
    }
    freeaddrinfo(res);
    ESP_LOGI(TAG, "DNS lookup succeeded for %s", FIREBASE_HOST);
    return 0;
}

static esp_err_t firebase_http_request(const char* url, const char* post_data) {
    if (check_dns() != 0) return ESP_ERR_HTTP_CONNECT;

    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = FIREBASE_ROOT_CA,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_PATCH);  // <-- use PATCH to merge timestamp entries
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return err;
}

int firebase_send_data(const char* path, float value) {
    char url[256];
    snprintf(url, sizeof(url), "https://%s/%s.json", FIREBASE_HOST, path);

    cJSON *root = cJSON_CreateNumber(value);
    char *post_data = cJSON_PrintUnformatted(root);

    esp_err_t err = firebase_http_request(url, post_data);

    cJSON_Delete(root);
    free(post_data);

    return (err == ESP_OK) ? 0 : -1;
}

// Send meter structure (Option 1 format)
int firebase_send_meter(const char* device, float voltage, float current, float active_power,
                        float reactive_power, float power_factor, float frequency, float total_energy) {
    char url[256];
    snprintf(url, sizeof(url), "https://%s/%s.json", FIREBASE_HOST, device);

    time_t now;
    struct tm timeinfo;
    char timestamp[32];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", &timeinfo);

    cJSON *meter_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(meter_data, "voltage", voltage);
    cJSON_AddNumberToObject(meter_data, "current", current);
    cJSON_AddNumberToObject(meter_data, "active_power", active_power);
    cJSON_AddNumberToObject(meter_data, "reactive_power", reactive_power);
    cJSON_AddNumberToObject(meter_data, "power_factor", power_factor);
    cJSON_AddNumberToObject(meter_data, "frequency", frequency);
    cJSON_AddNumberToObject(meter_data, "total_energy", total_energy);


    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, timestamp, meter_data);

    char *post_data = cJSON_PrintUnformatted(root);
    esp_err_t err = firebase_http_request(url, post_data);

    cJSON_Delete(root);
    free(post_data);

    return (err == ESP_OK) ? 0 : -1;
}