#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

typedef struct {
    float voltage;
    float current;
    float active_power;
    float reactive_power;
    float power_factor;
    float frequency;
    float total_energy;
} meter_data_t;

void modbus_init(void);

int read_dtsd1352(meter_data_t* data);

int read_adl400(meter_data_t* data);

#endif 