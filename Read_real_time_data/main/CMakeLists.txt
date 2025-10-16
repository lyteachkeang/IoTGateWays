idf_component_register(
    SRCS "Read_real_time_data.c" "modbus.c" "firebase.c" "gateway.c"
    INCLUDE_DIRS "."
    REQUIRES cJSON esp_http_client esp_wifi esp_netif nvs_flash driver 
)