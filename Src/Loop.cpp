extern "C"
{
#include "main.h"
}

#include "Loop.h"
#include "Command.h"
#include "Measure.h"
#include "stm32f3xx_hal.h"


void doLoop(void){
	int count = 0;
	char message[100];
	while (true){
	
		peekProcessCommand();

		HAL_Delay(10);
		count++;

	}
}
