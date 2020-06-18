extern "C"
{
#include "main.h"
}

#include "Loop.h"
#include "Command.h"
#include "Measure.h"
#include "Waveform.h"
#include "stm32f3xx_hal.h"

#define TARGET_HD 0.05

void doLoop(void){
	int count = 0;
	char message[100];
	float harmonicDistortion;
	while (true){
	
		peekProcessCommand();

		HAL_Delay(10);
		count++;
		harmonicDistortion=get3HD();
		adjustPower(TARGET_HD - harmonicDistortion);
	}
}
