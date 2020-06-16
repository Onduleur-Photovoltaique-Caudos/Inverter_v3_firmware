#pragma once
#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif


void initializeTemperatureSensors();


#ifdef __cplusplus
}
#endif

#define TEMPERATURE_SENSOR_COUNT 8

void doTemperatureAcquisitionStep();

float getTemp(int i);
float acquireTemp(int i);

#endif
