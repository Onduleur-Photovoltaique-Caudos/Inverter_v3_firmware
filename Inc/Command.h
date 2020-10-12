#pragma once

#ifdef __cplusplus
typedef enum {
	eNormal,
	eWaitConfirmation,
	eOver,
	eEmergency
} t_breakerState;

void logMessage(const char * message);
t_breakerState getBreakerState();
void setBreaker(t_breakerState newState,float f_IIN,float max_IIN, float f_IOUT, float max_IOUT);
void displayRecordedMeasurements();
char * my_itoa(int n, int maxVal = 100000);

bool isRun();
void doRunStop();
void doRunEnable();
void doRunJustBooted();
void doRunLowVoltage();
void doRunNormalVoltage();
bool doAC(bool newACState);
bool setACState(int newACState);
bool isAC();

void setRt(int val);
void setZ1(int val);
void setZ2(int val);
void setD1(int val);
void setD2(int val);
void setT1(int val);
void setT2(int val);

void sendSerial(const char* message);
void peekProcessCommand(void);

#endif


#ifdef __cplusplus
extern "C"
{
#endif
	void initializeCommand();
	void doEmergencyStop();
#ifdef __cplusplus
}
#endif