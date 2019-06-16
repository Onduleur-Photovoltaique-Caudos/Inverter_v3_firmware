#pragma once


bool isRun();
void doRunStop();
void doRunEnable();
void doRunJustBooted();
void doRunLowVoltage();
void doRunNormalVoltage();

void setRt(int val);
void setZ1(int val);
void setZ2(int val);
void setD1(int val);
void setD2(int val);

void sendSerial(char* message);
void peekProcessCommand(void);


#ifdef __cplusplus
extern "C"
{
#endif
	void initializeCommand();
#ifdef __cplusplus
}
#endif