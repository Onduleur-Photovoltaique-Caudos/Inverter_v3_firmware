extern "C"
{
#include "main.h"
}

#include "Measure.h"
#include "Command.h"
#include "Loop.h"
#include "stm32f3xx_ll_adc.h"
#include "MedianFilter.h"
#include "gpio.h"
#include <cmath>
#include <stdio.h>
#include "hrtim.h"
#include "tim.h"
#include "Waveform.h"
#include "Command.h"


int g_MeasurementNumber = 0;

uint16_t g_ADCBufferM[ADC_BUFFERM_LENGTH*2];
uint16_t * pM_VIN = &g_ADCBufferM[0];
uint16_t * pM_V175 = &g_ADCBufferM[1];
uint16_t * pM_V225 = &g_ADCBufferM[2];
uint16_t * pM_IOUT = &g_ADCBufferM[3];
uint16_t * pM_IHFL = &g_ADCBufferM[4];
uint16_t * pM_IH1 = &g_ADCBufferM[5];
uint16_t * pM_VOUT1 = &g_ADCBufferM[6];
uint16_t * pM_IH2 = &g_ADCBufferM[7];
uint16_t * pM_VOUT2 = &g_ADCBufferM[8];
uint16_t * pM_IIN = &g_ADCBufferM[9];
uint16_t * pM_Temp = &g_ADCBufferM[10];
uint16_t * pM_I175 = &g_ADCBufferM[11];
uint16_t * pM_Vref = &g_ADCBufferM[12];
uint16_t * pM_I225 = &g_ADCBufferM[13];


// store previous values
unsigned short g_ADCOld[14];
uint16_t * oM_VIN = &g_ADCOld[0];
uint16_t * oM_V175 = &g_ADCOld[1];
uint16_t * oM_V225 = &g_ADCOld[2];
uint16_t * oM_IOUT = &g_ADCOld[3];
uint16_t * oM_IHFL = &g_ADCOld[4];
uint16_t * oM_IH1 = &g_ADCOld[5];
uint16_t * oM_VOUT1 = &g_ADCOld[6];
uint16_t * oM_IH2 = &g_ADCOld[7];
uint16_t * oM_VOUT2 = &g_ADCOld[8];
uint16_t * oM_IIN = &g_ADCOld[9];
uint16_t * oM_Temp = &g_ADCOld[10];
uint16_t * oM_I175 = &g_ADCOld[11];
uint16_t * oM_Vref = &g_ADCOld[12];
uint16_t * oM_I225 = &g_ADCOld[13];
 
//#define VALIDATE(newValue,oldValue) ((newValue) != 0 ? (oldValue) = ((newValue) > 0 && (newValue) > (oldValue)*9/10 && (newValue) < (oldValue)*11/10? (newValue):(oldValue)):(oldValue))
#define VALIDATE(newValue,oldValue) ((oldValue) = (newValue))


#define ADC_STEPS 4096
#define ADC_FULL_MEASURE_MV (VREFINT_CAL_VREF * 1.0f)
#define RESISTOR_400V 4422
#define RESISTOR_200V 4200
#define RESISTOR_3V 22

unsigned short * pRefInt = (((uint16_t*)((uint32_t)0x1FFFF7BAU)));
unsigned short sRefInt;
const float mvFactor0 = ADC_FULL_MEASURE_MV / ADC_STEPS;
const float mvFactor1 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_400V / RESISTOR_3V;
const float mvFactor2 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_200V / RESISTOR_3V;
const float iOffset = 2520; // standard offset for ACS712 depends on supply voltage

//2568 unit 1

const float iDivider = 9.0f / 6.8f;  // values of resistor bridge for current
const float iFactor1 = 1.0f / 100.0f;    // for 20A model 100mV/A
const float iFactor2 = 1.0f / 185.0f;    // for 5A model 185mv/A    

volatile float fM_VIN, fM_V225, fM_IHFL, fM_VOUT1, fM_VOUT2, fM_Temp, fM_Vref;
volatile float mvCorrectionFactor = 1.0;
volatile float fM_V175, fM_IOUT, fM_IH1, fM_IH2, fM_IIN, fM_I175, fM_I225;

float ratioV225 = 160.0/400;
float ratioV175 = 210.0/400;

#define period  16000
static unsigned short compare_225 = period / 2;
static unsigned short compare_175 = period / 2;

//pid factor
#define P_factorInv 16

bool stopped_225;
bool stopped_175;

#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)

float getRatioV225()
{
	return ratioV225;
}
void setV1(float val){
	ratioV225 = val / 400.0;
}

float getRatioV175()
{
	return ratioV175;
}
void setV2(float val)
{
	ratioV175 = val / 400.0;
}

static bool stoppedZVS;

void doStopZVS(bool newValue)
{
	//stoppedZVS = newValue;
}

#define START_VOLTAGE 80
#define MAX_INPUT_CURRENT 10
#define MAX_OUTPUT_CURRENT 20

#define RECORD_COUNT 5
typedef enum {
	eNotTripped,
	eTripped,
	eTrippedNotRecording
} eTrippedState;

volatile eTrippedState trippedState;
volatile static int trippedRecordCountdown;

void reEnableTripRecording()
{
	trippedState = eNotTripped;
}
void checkOvercurrent(float f_IIN,float f_IOUT){
	if (trippedState == eTripped) {
		if (--trippedRecordCountdown == 0){
			trippedState = eTrippedNotRecording;
			queueMessage(Message(eMCCurrentMeasurementsDump));
		}
	}
	if (f_IIN < MAX_INPUT_CURRENT && f_IOUT < MAX_OUTPUT_CURRENT){
		if (getBreakerState()==eWaitConfirmation){
			setBreaker(eNormal, f_IIN, MAX_INPUT_CURRENT, f_IOUT, MAX_OUTPUT_CURRENT);
		}
		return;
	}
	if (getBreakerState() == eNormal) {
		setBreaker(eWaitConfirmation, f_IIN, MAX_INPUT_CURRENT, f_IOUT, MAX_OUTPUT_CURRENT);
		return;
	}
	if (f_IIN / 1.5f > MAX_INPUT_CURRENT || f_IOUT / 1.5f < MAX_OUTPUT_CURRENT) {
		//emergency stop
		setACState(false);
		//setRt(0);
		setBreaker(eEmergency, f_IIN, MAX_INPUT_CURRENT, f_IOUT, MAX_OUTPUT_CURRENT);
		if (!trippedState == eNotTripped) {
			trippedState = eTripped;
			trippedRecordCountdown = RECORD_COUNT;
		}
	} else { // we stop a next zero crossing
		setACState(false);
		setBreaker(eOver, f_IIN, MAX_INPUT_CURRENT, f_IOUT, MAX_OUTPUT_CURRENT);
		if (!trippedState == eNotTripped) {
			trippedState = eTripped;
			trippedRecordCountdown = RECORD_COUNT;
		}
	}
}
void adjust_225_175(float inputVoltage)
{
	float fM_VIN_save = fM_VIN;
	if (inputVoltage < START_VOLTAGE * 1000.0) {
		doRunLowVoltage();
	} else {
		doRunNormalVoltage();
	}

	float target_175 = fM_VIN * ratioV175;
	float target_225 = fM_VIN * ratioV225;
	
	float diff_175 = (target_175 - fM_V175) / fM_VIN;
	float diff_225 = (target_225 - fM_V225) / fM_VIN;
	int diff;
	
	HRTIM_CompareCfgTypeDef compareCfg;

	// 225V TE1 PC8 test pin P1
	// 225 Timer E1 -> PC8  C_225 -> P1 -> U1(2) -> P23(1) -> Q1 L1 Q3 D3 
	diff = diff_225 * period /P_factorInv ;
	int sign_or_0 = (0 < diff) - (diff < 0);
	diff =  (diff ? sign_or_0 : 0) + diff * max(compare_225, 96) / 40000;

	compareCfg.CompareValue = min(period - 96, max(95, compare_225 - diff));
	//compareCfg.CompareValue = period / 3;  // debug: force it TODO remove
	compare_225 = compareCfg.CompareValue; // remember previous value;

	if(compareCfg.CompareValue < 96 || stoppedZVS)
	{
		 // force inactive
		stopped_225 = true;
		HAL_HRTIM_WaveformOutputStop(&hhrtim1,
			HRTIM_OUTPUT_TE1);
		HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_E,
			HRTIM_OUTPUT_TE1,
			HRTIM_OUTPUTLEVEL_INACTIVE);
	} else if (compareCfg.CompareValue >= period - 96) { // force active
		compareCfg.CompareValue = period - 1;
		HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_E,
			HRTIM_COMPAREUNIT_1,
			&compareCfg);
	} else {
		if (stopped_225) {
			HAL_HRTIM_WaveformOutputStart(&hhrtim1,
				HRTIM_OUTPUT_TE1);
			stopped_225 = false;
		}
		HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_E,
			HRTIM_COMPAREUNIT_1,
			&compareCfg);
	}
	
		// 175V TC1 PB12  test pin P2
		// 175 Timer C1 -> PB12 C_175  -> P2 -> U1(1) -> P24(1) -> Q2 L2 Q4 D4 
	diff = diff_175 * period / P_factorInv;
	sign_or_0 = (0 < diff) - (diff < 0);
	diff =  (diff ? sign_or_0 : 0) + diff *  max(compare_175, 96) / 40000;

	compareCfg.CompareValue = min(period - 96, max(95, compare_175 + diff));
	//compareCfg.CompareValue = period / 3;
	compare_175 = compareCfg.CompareValue;

	if (compareCfg.CompareValue < 96 || stoppedZVS) {
		stopped_175 = true;
		HAL_HRTIM_WaveformOutputStop(&hhrtim1,
			HRTIM_OUTPUT_TC1);
		HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_C,
			HRTIM_OUTPUT_TC1,
			HRTIM_OUTPUTLEVEL_INACTIVE);
	} else if (compareCfg.CompareValue >= period - 96) { // force active
		if(stopped_175)	{
			HAL_HRTIM_WaveformOutputStart(&hhrtim1,
				HRTIM_OUTPUT_TC1);
			stopped_175 = false;
		}
		compareCfg.CompareValue = period - 1;
		HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_C,
			HRTIM_COMPAREUNIT_1,
			&compareCfg);
	} else {
		if (stopped_175) {
			HAL_HRTIM_WaveformOutputStart(&hhrtim1,
				HRTIM_OUTPUT_TC1);
			stopped_175 = false;
		}
		HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_C,
			HRTIM_COMPAREUNIT_1,
			&compareCfg);
	}
}

bool bErrorDMA;
int  bErrorADC;

void   HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) // data overrun
{
	if (hadc->ErrorCode == HAL_ADC_ERROR_DMA) {
		bErrorDMA = true;
	} else if (hadc->ErrorCode == HAL_ADC_ERROR_OVR) {
		bErrorADC = 1;
	} else {
		bErrorADC = 2;
	}
}


static int countEOC;
static int countADCPeriod;
static bool bADCPeriodStatsStarted;
static int moyADCPeriod;
static int minADCPeriod = 1000;
static int maxADCPeriod;

void resetADCPeriodCounters(){
	countADCPeriod = 0;
	moyADCPeriod=0;
	minADCPeriod = 1000;
	maxADCPeriod=0;
}

static int pointerRecordMeasurement;
#define MAX_RECORD_MEASUREMENT 50
static volatile float measurementRecord[MAX_RECORD_MEASUREMENT];

void doRecordMeasurement(float newMeasure)
{
	measurementRecord[pointerRecordMeasurement++] = newMeasure;
	if (pointerRecordMeasurement >= MAX_RECORD_MEASUREMENT){
		pointerRecordMeasurement = 0; // implement a circular buffer;
	}
}

static int startReadRecords;
int getMeasurementRecordCount()
{	
	startReadRecords = pointerRecordMeasurement + 1;
	return MAX_RECORD_MEASUREMENT;
}
float getMeasurementNextRecord()
{
	if (startReadRecords >= MAX_RECORD_MEASUREMENT) {
		startReadRecords = 0;
	}
	return measurementRecord[startReadRecords++];
}


static int counterStatsMeasurement;
#define MAX_BUCKETS 11
static int bucketsStatsMeasurement[MAX_BUCKETS];
static void resetStatsMeasurement(){
	counterStatsMeasurement = 0;
	for (int i = 0; i < MAX_BUCKETS; i++) {
		bucketsStatsMeasurement[i] = 0;
	}
}
static void doStatsMeasurement(float voltage)
{
	counterStatsMeasurement++;
	if (voltage < 1.0){
		bucketsStatsMeasurement[0]++;
	} else if (voltage > 9000.0){
		bucketsStatsMeasurement[MAX_BUCKETS - 1]++;
	} else {
		if (voltage > 8000) {
			voltage++;
		} else if (voltage < 1000){
			voltage++;
		}
		int index = (voltage / 1000.0) + 1;
		bucketsStatsMeasurement[index]++;
	}
}

char * getMeasureStats(int what, char * message){
	switch (what) {
	case 0:
		sprintf(message, "%d\r\n", get_us_DWT(0));
		start_us_DWT(0);
		break;
	case 1:
		sprintf(message,
			"moyADCPer: %d min: %d max: %d\r\n",
			moyADCPeriod/countADCPeriod,
			minADCPeriod,
			maxADCPeriod);
		resetADCPeriodCounters();
		break;
	case 2:
		sprintf(message,
			"Bucket 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d 9:%d 10:%d\r\n",
			bucketsStatsMeasurement[0],
			bucketsStatsMeasurement[1],
			bucketsStatsMeasurement[2],
			bucketsStatsMeasurement[3],
			bucketsStatsMeasurement[4],
			bucketsStatsMeasurement[5],
			bucketsStatsMeasurement[6],
			bucketsStatsMeasurement[7],
			bucketsStatsMeasurement[8],
			bucketsStatsMeasurement[9],
			bucketsStatsMeasurement[10]);
		resetStatsMeasurement();
	default:
		break;
	}
	return message;
}

static volatile unsigned long long measureCount;
static volatile bool doneADC;
unsigned long long getMeasureCount()
{
	return measureCount;
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adcHandle)
{// end of DMA
	measureCount++;
	if (isADC_EOC(adcHandle)){
		//doPsenseToggle();
		//Error_Handler();// we should not be there
		countEOC++;
		return;
	}
	if (adcHandle == &hadc1) {
		 // 3 microseconds in optimized mode 8us in debug
		if(bADCPeriodStatsStarted)
		{
			int ADCPeriod = get_us_DWT(1);
			start_us_DWT(1);
			moyADCPeriod += ADCPeriod;
			countADCPeriod++;
			if (minADCPeriod > ADCPeriod) {
				minADCPeriod = ADCPeriod;
			}
			if (maxADCPeriod < ADCPeriod) {
				maxADCPeriod = ADCPeriod;
			}
		} else
		{
			start_us_DWT(1);
			resetADCPeriodCounters();
			bADCPeriodStatsStarted = true;
		}
		countEOC = 0;
		g_MeasurementNumber++;

		VALIDATE(*pM_VIN, *oM_VIN);
		fM_VIN = (*oM_VIN) *mvFactor1 * mvCorrectionFactor;  		// test at P21 -- ok pas de bruit
		VALIDATE(*pM_V225, *oM_V225);
		fM_V225 = (*oM_V225) *mvFactor2 * mvCorrectionFactor;  	// test at P3  -- ok pas de bruit
		VALIDATE(*pM_IHFL, *oM_IHFL);
		fM_IHFL = ((*oM_IHFL) - iOffset)  *iDivider * mvCorrectionFactor;  // test at U11 pin 1 -- error lit sur VOUT1
		VALIDATE(*pM_VOUT1, *oM_VOUT1);
		fM_VOUT1 = (*oM_VOUT1) *mvFactor1; 			// ADC1 IN11 test at P20 -- OK
		VALIDATE(*pM_VOUT2, *oM_VOUT2);
		fM_VOUT2 = (*oM_VOUT2) *mvFactor1; 			// test at P18 -- OK
		VALIDATE(*pM_Temp, *oM_Temp);
		fM_Temp = ((*TEMP30_CAL_ADDR) - (*oM_Temp)* mvCorrectionFactor) * 80.0f / ((*TEMP30_CAL_ADDR) - (*TEMP110_CAL_ADDR)) + 30; 
		; 
		VALIDATE(*pM_Vref, *oM_Vref);
		fM_Vref = (*oM_Vref) * mvFactor0;
		mvCorrectionFactor =  0.01f*(* VREFINT_CAL_ADDR) / (*oM_Vref) + 0.99f * mvCorrectionFactor;
		if (mvCorrectionFactor > 2.0 || mvCorrectionFactor<0.5) {
			mvCorrectionFactor = 1.13;
		}
		VALIDATE(*pM_V175, *oM_V175);
		fM_V175 = (*oM_V175) *mvFactor2 * mvCorrectionFactor;  			// test at P4  -- ok 
		//doStatsMeasurement(fM_V175);
		VALIDATE(*pM_IOUT, *oM_IOUT);
		fM_IOUT = ((*oM_IOUT)*mvFactor0  * mvCorrectionFactor * iDivider - iOffset) * iFactor1;       	// test at P13 pin2 (range 5V) -- bruite error, pas connecte essayer R43 R44
		doRecordMeasurement(fM_IOUT);
		VALIDATE(*pM_IH1, *oM_IH1);
		fM_IH1 = ((*oM_IH1) - iOffset) *iDivider* mvCorrectionFactor;     		// test at U20 pin1 -- error lit sur IOUT
		VALIDATE(*pM_IH2, *oM_IH2);
		fM_IH2 = ((*oM_IH2) - iOffset) *iDivider* mvCorrectionFactor;     		// test at U17 pin1 -- OK
		VALIDATE(*pM_IIN, *oM_IIN);
		fM_IIN = ((*oM_IIN)*mvFactor0  * mvCorrectionFactor * iDivider - iOffset) * iFactor1;     	// test at P16 pin2 range 5V --OK
		VALIDATE(*pM_I175, *oM_I175);
		fM_I175 = ((*oM_I175)*mvFactor0  * mvCorrectionFactor * iDivider - iOffset) * iFactor2;      	// test at P10 pin2 range 5V --OK
		VALIDATE(*pM_I225, *oM_I225);
		fM_I225 = ((*oM_I225)*mvFactor0  * mvCorrectionFactor * iDivider - iOffset) * iFactor2;      	// test at P9 pin2 range 5V -- OK

//		checkOvercurrent(fM_IIN, fM_IOUT);
		adjust_225_175(fM_VIN);
		doSecondHalfStep();
		doneADC = true;
	} else {// end of adc1 processing
		Error_Handler();
	}
}

float getInputVoltage(){
	return fM_VIN;
}

static volatile bool bPrepareAC;
bool setPrepareAC(bool newValue)
{
	bool bPreviousValue = bPrepareAC;
	bPrepareAC = newValue;
	return bPreviousValue;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim1) {
		// once per 50Hz period
		if(bPrepareAC){
			doStartAC();
			bPrepareAC = false;
		}
			doSyncSerialToggle();
			//doResetWaveform();
	} else if (htim == &htim3) {

		Error_Handler();
		//not used doWaveformStep();
	} else {
		Error_Handler();
	}
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) // same as HAL_TIM_PWM_PulseFinishedCallback
{
	if (htim == &htim15) {
		//executeSetRt();
	} else if(htim == &htim1){
		//
		// this happens at the start of ADC acquisition
		// 9us before XferCplt (release) or 15us (debug)
		//

		if(0 && doneADC)
		{
			// here some processing if needed
			delay_us_DWT(1);
			return;
		}
	} else if (htim == &htim3) { //htim3 triggers the waveform step
	
		delay_us_DWT(2); // need 1us delay to avoir glitch
		doWaveformStep(); // 40us
	} else if (htim == &htim2){
// not used
	} else  {
		Error_Handler(); // another timer ?
	}
}

float getIIN(){
	return fM_IIN;
}

float getVIN(){
	return fM_VIN;
}