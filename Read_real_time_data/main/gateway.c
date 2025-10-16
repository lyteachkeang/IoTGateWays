#include <stdio.h>     
#include <time.h>
#include "gateway.h"
#include "firebase.h"
#include "modbus.h"
#include "esp_log.h"

static const char *TAG = "GATEWAY";

void gateway_init(void) {
    modbus_init(); 
    ESP_LOGI(TAG, "Gateway initialized");
}

int gateway_read_all(gateway_t* gateway) {
    int status_adl = read_adl400(&gateway->adl400);
    int status_dtsd = read_dtsd1352(&gateway->dtsd1352);

    if (status_adl != 0) ESP_LOGW(TAG, "Failed to read ADL400");
    if (status_dtsd != 0) ESP_LOGW(TAG, "Failed to read DTSD1352");

    return (status_adl == 0 && status_dtsd == 0) ? 0 : -1;
}

void gateway_log_send(const char* gateway_name, gateway_t* gateway) {
    char adl_path[64];
    char dtsd_path[64];

    snprintf(adl_path, sizeof(adl_path), "%s/ADL400", gateway_name);
    snprintf(dtsd_path, sizeof(dtsd_path), "%s/DTSD1352", gateway_name);

    ESP_LOGI(TAG, "Sending ADL400 data to Firebase");
    firebase_send_meter(adl_path,
                        gateway->adl400.voltage,
                        gateway->adl400.current,
                        gateway->adl400.active_power,
                        gateway->adl400.reactive_power,
                        gateway->adl400.power_factor,
                        gateway->adl400.frequency,
                        gateway->adl400.total_energy);

    ESP_LOGI(TAG, "Sending DTSD1352 data to Firebase");
    firebase_send_meter(dtsd_path,
                        gateway->dtsd1352.voltage,
                        gateway->dtsd1352.current,
                        gateway->dtsd1352.active_power,
                        gateway->dtsd1352.reactive_power,
                        gateway->dtsd1352.power_factor,
                        gateway->dtsd1352.frequency,
                        gateway->dtsd1352.total_energy);
}