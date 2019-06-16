extern "C"
{
#include "main.h"
}

#include "Measure.h"
#include "MedianFilter.h"
#include "gpio.h"
#include "math.h"
#include "hrtim.h"
#include "Waveform.h"
#include "Command.h"

int g_MeasurementNumber = 0;

#if 0
uint16_t g_ADCBuffer1[ADC_BUFFER1_LENGTH];
uint16_t * pM_VIN = &g_ADCBuffer1[0];
uint16_t * pM_V225 = &g_ADCBuffer1[1];
uint16_t * pM_IHFL = &g_ADCBuffer1[2];
uint16_t * pM_VOUT1 = &g_ADCBuffer1[3];
uint16_t * pM_VOUT2 = &g_ADCBuffer1[4];
uint16_t * pM_Temp = &g_ADCBuffer1[5];

uint16_t g_ADCBuffer2[ADC_BUFFER2_LENGTH];
uint16_t * pM_V175 = &g_ADCBuffer2[0];
uint16_t * pM_IOUT = &g_ADCBuffer2[1];
uint16_t * pM_IH1 = &g_ADCBuffer2[2];
uint16_t * pM_IH2 = &g_ADCBuffer2[3];
uint16_t * pM_IIN = &g_ADCBuffer2[4];
uint16_t * pM_I175 = &g_ADCBuffer2[5];
uint16_t * pM_I225 = &g_ADCBuffer2[5];

#else
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

#endif
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
 
#define VALIDATE(newValue,oldValue) ((newValue) != 0 ? (oldValue) = ((newValue) > 0 && (newValue) > (oldValue)*9/10 && (newValue) < (oldValue)*11/10? (newValue):(oldValue)):(oldValue))


#define ADC_STEPS 4096
#define ADC_FULL_MEASURE_MV 3300.0
#define RESISTOR_400V 4422
#define RESISTOR_200V 2222
#define RESISTOR_3V 22

const float mvFactor1 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_400V / RESISTOR_3V;
const float mvFactor2 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_200V / RESISTOR_3V;
const float iOffset = 2206;
const float iFactor = 1.5/128;

volatile float fM_VIN, fM_V225, fM_IHFL, fM_VOUT1, fM_VOUT2, fM_Temp, fM_Vref;
volatile float fM_V175, fM_IOUT, fM_IH1, fM_IH2, fM_IIN, fM_I175, fM_I225;

float ratioV225 = 150.0/400;
float ratioV175 = 250.0/400;

#define period  16000
static unsigned short compare_225 = period / 2;
static unsigned short compare_175 = period / 2;

//pid factor
#define P_factorInv 8

bool stopped_225;
bool stopped_175;

#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)

void setV1(float val){
	ratioV225 = val / 400.0;
}

void setV2(float val)
{
	ratioV175 = val / 400.0;
}

static bool forceStop;

void doForceStop(bool newValue) {
	forceStop = newValue;
}

#define START_VOLTAGE 80
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
	compare_225 = compareCfg.CompareValue; // remember previous value;

	if (compareCfg.CompareValue < 96 || forceStop) { // force inactive
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
	diff = diff_175 * period / P_factorInv *4;
	sign_or_0 = (0 < diff) - (diff < 0);
	diff =  (diff ? sign_or_0 : 0) + diff *  max(compare_175, 96) / 40000;

	compareCfg.CompareValue = min(period - 96, max(200/*95*/, compare_175 + diff));
	compare_175 = compareCfg.CompareValue;

	if (compareCfg.CompareValue < 96 || forceStop) {
		stopped_175 = true;
		HAL_HRTIM_WaveformOutputStop(&hhrtim1,
			HRTIM_OUTPUT_TC1);
		HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_C,
			HRTIM_OUTPUT_TC1,
			HRTIM_OUTPUTLEVEL_INACTIVE);
	} else if (false && compareCfg.CompareValue >= period - 96) { // force active
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

static int counterStatsVoltage;
#define MAX_BUCKETS 11
static int bucketsStatsVoltage[MAX_BUCKETS];
static void resetStatsVoltage(){
	counterStatsVoltage = 0;
	for (int i = 0; i < MAX_BUCKETS; i++) {
		bucketsStatsVoltage[i] = 0;
	}
}
static void doStatsVoltage(double voltage)
{
	counterStatsVoltage++;
	if (voltage < 1.0){
		bucketsStatsVoltage[0]++;
	} else if (voltage > 9000.0){
		bucketsStatsVoltage[MAX_BUCKETS-1]++;
	} else {
		if (voltage > 8000) {
			voltage++;
		} else if (voltage < 1000){
			voltage++;
		}
		int index = (voltage / 1000.0) + 1;
		bucketsStatsVoltage[index]++;
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
			bucketsStatsVoltage[0],
			bucketsStatsVoltage[1],
			bucketsStatsVoltage[2],
			bucketsStatsVoltage[3],
			bucketsStatsVoltage[4],
			bucketsStatsVoltage[5],
			bucketsStatsVoltage[6],
			bucketsStatsVoltage[7],
			bucketsStatsVoltage[8],
			bucketsStatsVoltage[9],
			bucketsStatsVoltage[10]);
		resetStatsVoltage();
	default:
		break;
	}
	return message;
}

static volatile unsigned long long measureCount;

unsigned long long getMeasureCount()
{
	return measureCount;
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adcHandle)
{// end of DMA
	measureCount++;
	if (isADC_EOC(adcHandle)){
		doPsenseToggle();
		countEOC++;
		return;
	}
	if (adcHandle == &hadc1) { // 3 microseconds in optimized mode 8us in debug
		if (bADCPeriodStatsStarted) {
			int ADCPeriod = get_us_DWT(1);
			start_us_DWT(1);
			moyADCPeriod += ADCPeriod;
			countADCPeriod++;
			if (minADCPeriod > ADCPeriod){
				minADCPeriod = ADCPeriod;
			}
			if (maxADCPeriod < ADCPeriod) {
				maxADCPeriod = ADCPeriod;
			}
		} else {
			start_us_DWT(1);
			resetADCPeriodCounters();
			bADCPeriodStatsStarted = true;
		}
		static int countWithinSegment;
		static bool bStopped = true;
		countWithinSegment++;
		countWithinSegment %= 4;
		if (bStopped) {
			bStopped = !isRun();
		} else {
		// debug mode: inhibit sinusoidal output
#define DO_NORMAL_WAVEFORM 1
#if DO_NORMAL_WAVEFORM
			if (0 == countWithinSegment){
				bool bZeroCrossing = doNextWaveformSegment();
				if (bZeroCrossing && !isRun()) {
					bStopped = true;
				}
			}
#endif
		}
		doSyncSerialOn(); // 45-70 us debug, 15 us release
		countEOC = 0;
		g_MeasurementNumber++;

		VALIDATE(*pM_VIN, *oM_VIN);
		fM_VIN = (*oM_VIN) *mvFactor1;

		VALIDATE(*pM_V225, *oM_V225);
		fM_V225 = (*oM_V225) *mvFactor2;

		VALIDATE(*pM_IHFL, *oM_IHFL);
		fM_IHFL = ((*oM_IHFL)-iOffset) *iFactor;
		VALIDATE(*pM_VOUT1, *oM_VOUT1);
		fM_VOUT1 = (*oM_VOUT1) *mvFactor1;
		VALIDATE(*pM_VOUT2, *oM_VOUT2);
		fM_VOUT2 = (*oM_VOUT2) *mvFactor1;
		VALIDATE(*pM_Temp, *oM_Temp);
		fM_Temp = (*oM_Temp) * 1; 
		VALIDATE(*pM_Vref, *pM_Vref);
		fM_Vref = (*oM_Vref) * 1;
		// ADC2
		VALIDATE(*pM_V175, *oM_V175);
		fM_V175 = (*oM_V175) *mvFactor2;
		doStatsVoltage(fM_V175);
		VALIDATE(*pM_IOUT, *oM_IOUT);
		fM_IOUT = ((*oM_IOUT)-iOffset) *iFactor; 
		VALIDATE(*pM_IH1, *oM_IH1);
		fM_IH1 = ((*oM_IH1)-iOffset) *iFactor; 
		VALIDATE(*pM_IH2, *oM_IH2);
		fM_IH2 = ((*oM_IH2)-iOffset) *iFactor;
		VALIDATE(*pM_IIN, *oM_IIN);
		fM_IIN = ((*oM_IIN)-iOffset) *iFactor; 
		VALIDATE(*pM_I175, *oM_I175);
		fM_I175 = ((*oM_I175)-iOffset) *iFactor; 
		VALIDATE(*pM_I225, *oM_I225);
		fM_I225 = ((*oM_I225)-iOffset) *iFactor; 
		adjust_225_175(fM_VIN);
		doSyncSerialOff();
	}
	
}
