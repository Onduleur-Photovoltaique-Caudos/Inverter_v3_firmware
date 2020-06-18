#pragma once

typedef enum {
	eNormal,
	eOver,
	eEmergency
} t_breakerState;

void setBreaker(t_breakerState newState,float f_IIN, float f_IOUT);
int getPower();
void adjustPower(float adjustment);

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


#ifdef __cplusplus
extern "C"
{
#endif
	void initializeCommand();
#ifdef __cplusplus
}
#endif