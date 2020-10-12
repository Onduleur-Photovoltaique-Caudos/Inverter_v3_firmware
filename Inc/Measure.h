#pragma once
#ifndef __MEASURE_H
#define __MEASURE_H

#include "adc.h"

#define ADC_BUFFER1_LENGTH 7
extern uint16_t g_ADCBuffer1[];

#define ADC_BUFFER2_LENGTH 7
extern uint16_t g_ADCBuffer2[];

#define ADC_BUFFERM_LENGTH 7
extern uint16_t g_ADCBufferM[];

extern volatile float fM_VIN, fM_V225, fM_IHFL, fM_VOUT1, fM_VOUT2, fM_Temp, fm_Vref, mvCorrectionFactor;
extern volatile float fM_V175, fM_IOUT, fM_IH1, fM_IH2, fM_IIN, fM_I175, fM_I225;


extern int g_MeasurementNumber;

float getRatioV225();
float getRatioV175();
void setV1(float val);
void setV2(float val);
void doStopZVS(bool newValue);
unsigned long long getMeasureCount();
char * getMeasureStats(int what,char* message);
int getMeasurementRecordCount();
float getMeasurementNextRecord();
void reEnableTripRecording();

float getIIN();

float getVIN();

#ifdef __cplusplus
extern "C"
{
#endif
	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adcHandle);
#ifdef __cplusplus
 }
#endif



#endif
