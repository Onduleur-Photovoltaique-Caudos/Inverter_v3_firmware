extern "C"
{
#include "main.h"
}

#include "Loop.h"
#include "Command.h"
#include "Measure.h"
#include "Waveform.h"
#include "Temperature.h"
#include "stm32f3xx_hal.h"


const char* messageTable[]{"Overcurrent","Emergency","queueOverflow","currentMeasurementsDump"};
#define MAX_MESSAGES 10
volatile unsigned short messagePointer;
volatile unsigned short nextMessage=1;

Message messageList[MAX_MESSAGES];
void queueMessage(Message message){
	static bool hasOverflown;
	if ((nextMessage + 1) % MAX_MESSAGES == messagePointer) {
		 //overflow
		if(!hasOverflown)
		{
			messageList[nextMessage] =  Message(eMCQueueOverflow, 0, 0);
			hasOverflown = true;
		} 
	} else {
		hasOverflown = false;
		messageList[nextMessage++] = message;
	}
}

void processMessageQueue()
{
	if ((messagePointer + 1) % MAX_MESSAGES == nextMessage) { // nothing to print
		return;
	}
	if (messageList[messagePointer].code == eMCCurrentMeasurementsDump) {
		//displayRecordedMeasurements();
		void reEnableTripRecording();
		goto exitIncrement;
		}
#if 0
	logMessage("\r\n");
	logMessage(messageTable[messageList[messagePointer].code]); 
	logMessage("\tIin:");
	logMessage(my_itoa(messageList[messagePointer].val1,1000));
	logMessage("\tIout:");
	logMessage(my_itoa(messageList[messagePointer].val2,1000));
	logMessage("\r\n");
#endif
exitIncrement:
	messagePointer++;
}

void doLoop(void){
	int count = 0;
	char message[100];
	float harmonicDistortion;
	while (true){

		HAL_GPIO_WritePin(Disable_GPIO_Port, Disable_Pin, GPIO_PIN_RESET);
	
		peekProcessCommand();

		HAL_Delay(10);
		count++;
		doAdjustPower();
		doAdjustFanSpeed();
		processMessageQueue();
		//doTemperatureAcquisitionStep();
	}
}
