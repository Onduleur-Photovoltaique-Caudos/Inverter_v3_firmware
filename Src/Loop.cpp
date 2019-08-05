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
		//doNextWaveformSegment();
	#ifdef USE_SERIAL	
		peekProcessCommand();
	#endif
		HAL_Delay(10);
		count++;
		if (count == 99) {
			mutexStats = 1;
			count = 0;

			//getMeasureStats(2,message);
			//sendSerial(message);
		}
	}
}
