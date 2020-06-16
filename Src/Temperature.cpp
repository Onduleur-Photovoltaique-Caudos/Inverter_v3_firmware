#include "Temperature.h"

#include "i2c.h"
#include "stm32f3xx_hal_i2c.h"
#include "LM75.h"

TempI2C_LM75 * sensors[TEMPERATURE_SENSOR_COUNT];

void initializeTemperatureSensors()
{
	for (int i = 0; i < TEMPERATURE_SENSOR_COUNT; i++)
	{
		sensors[i] = new TempI2C_LM75(&hi2c1, TempI2C_LM75::baseAddress + i);
	}
}



static int which;
void doTemperatureAcquisitionStep()
{
	sensors[which++]->acquireTemp(true);
	if (which >= TEMPERATURE_SENSOR_COUNT) {
		which = 0;
	}
}

float getTemp(int i){
	if (i < TEMPERATURE_SENSOR_COUNT) {
		return sensors[i]->getTemp();
	} else {
		return 0.0f;
	}
}

// force reinit of the i2c register
float acquireTemp(int i){
	if (i < TEMPERATURE_SENSOR_COUNT) {
		return sensors[i]->acquireTemp(false);
	} else {
		return 0.0f;
	}
}