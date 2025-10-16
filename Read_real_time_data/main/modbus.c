#include "modbus.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

#define MB_PORT_NUM      UART_NUM_1
#define MB_TXD_PIN       27
#define MB_RXD_PIN       26
#define MB_DE_RE_PIN     32 
#define MB_BAUD_RATE     9600

static const char *TAG = "MODBUS";

static uint16_t parse_u16(const uint8_t *buf) { 
    return (buf[3] << 8) | buf[4]; 
}

static uint32_t parse_u32(const uint8_t *buf) {
    uint16_t hi = (buf[3] << 8) | buf[4];
    uint16_t lo = (buf[5] << 8) | buf[6];
    return ((uint32_t)hi << 16) | lo;
}

static uint16_t crc16(const uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++)
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

void modbus_init(void) {
    //Configure DE/RE pin for RS485 direction control
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MB_DE_RE_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE 
        
    gpio_config(&io_conf);
    gpio_set_level(MB_DE_RE_PIN, 0); // receive mode
    };

    const uart_config_t uart_config = {
        .baud_rate = MB_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_ERROR_CHECK(uart_driver_install(MB_PORT_NUM, 256, 256, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(MB_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, MB_TXD_PIN, MB_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static int modbus_request(uint8_t slave_addr, uint8_t func,
                          uint16_t reg_addr, uint16_t num_regs,
                          uint8_t *response, int max_len) {
            uint8_t req[8];
            req[0] = slave_addr;
            req[1] = func;
            req[2] = reg_addr >> 8;
            req[3] = reg_addr & 0xFF;
            req[4] = num_regs >> 8;
            req[5] = num_regs & 0xFF;
            uint16_t crc = crc16(req, 6);
            req[6] = crc & 0xFF;
            req[7] = crc >> 8;

    uart_flush(MB_PORT_NUM);

    gpio_set_level(MB_DE_RE_PIN, 1); 
    uart_write_bytes(MB_PORT_NUM, (const char *)req, 8);
    uart_wait_tx_done(MB_PORT_NUM, 100 / portTICK_PERIOD_MS);

    gpio_set_level(MB_DE_RE_PIN, 0); 
    int len = uart_read_bytes(MB_PORT_NUM, response, max_len, 200 / portTICK_PERIOD_MS);
    if (len <= 0) {
        ESP_LOGW(TAG, "No response from slave %d", slave_addr);
        return -1;
    }

    uint16_t resp_crc = (response[len - 1] << 8) | response[len - 2];
    uint16_t calc_crc = crc16(response, len - 2);
    if (resp_crc != calc_crc) {
        ESP_LOGW(TAG, "CRC error: slave %d", slave_addr);
        return -1;
    }
    return len;
}

int read_adl400(meter_data_t* data) {
    uint8_t response[32];
    int len;

    len = modbus_request(6, 0x03, 0x0061, 1, response, sizeof(response));
    if (len < 7) return -1;
    data->voltage = parse_u16(response) / 10.0f;

    len = modbus_request(6, 0x03, 0x0064, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->current = parse_u32(response) / 10000000.0f;

    len = modbus_request(6, 0x03, 0x0067, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->active_power = parse_u32(response) / 10000000.0f;

    len = modbus_request(6, 0x03, 0x006B, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->reactive_power = parse_u32(response) / 10000000.0f;

    len = modbus_request(6, 0x03, 0x0073, 1, response, sizeof(response));
    if (len < 7) return -1;
    data->power_factor = parse_u16(response) /  10000000.0f;
    len = modbus_request(6, 0x03, 0x0077, 1, response, sizeof(response));
    if (len < 7) return -1;
    data->frequency = parse_u16(response) / 100.0f;
    len = modbus_request(6, 0x03, 0x0200, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->total_energy = parse_u32(response) / 1000000.0f;

    return 0;
}

int read_dtsd1352(meter_data_t* data) {
    uint8_t response[32];
    int len;

    len = modbus_request(1, 0x03, 0x0061, 1, response, sizeof(response));
    if (len < 5) return -1;
    data->voltage = parse_u16(response) / 10.0f;


    len = modbus_request(1, 0x03, 0x0064, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->current = parse_u32(response) /10000000.0f;


    len = modbus_request(1, 0x03, 0x0164, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->active_power = parse_u32(response) /1000.0f;


    len = modbus_request(1, 0x03, 0x016C, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->reactive_power = parse_u32(response) / 1000.0f;

    
    len = modbus_request(1, 0x03, 0x017C, 1, response, sizeof(response));
    if (len < 5) return -1;
    data->power_factor = parse_u16(response) /1000.0f;

   
    len = modbus_request(1, 0x03, 0x0077, 1, response, sizeof(response));
    if (len < 5) return -1;
    data->frequency = parse_u16(response) / 100.0f;

    
    len = modbus_request(1, 0x03, 0x0200, 2, response, sizeof(response));
    if (len < 9) return -1;
    data->total_energy = parse_u32(response) / 1000000.0f;

    return 0;
}

