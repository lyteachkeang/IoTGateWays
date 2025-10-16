#ifndef GATEWAY_H
#define GATEWAY_H

#include "modbus.h"

typedef struct {
    meter_data_t adl400;
    meter_data_t dtsd1352;
} 
    gateway_t;

void gateway_init(void);

int gateway_read_all(gateway_t* gw);

void gateway_log_send(const char* gateway_name, gateway_t* gw);

#endif 