#ifndef FIREBASE_H
#define FIREBASE_H

#include <stdint.h>

void firebase_init(void);
int firebase_send_data(const char* path, float value);
int firebase_send_meter(const char* base_path, float voltage, float current, float active_power,
                       float reactive_power, float power_factor, float frequency, float total_energy);

#endif