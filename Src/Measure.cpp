#include "Measure.h"
#include "gpio.h"
#include "math.h"
#include "hrtim.h"

int g_MeasurementNumber = 0;

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

#define ADC_STEPS 4096
#define ADC_FULL_MEASURE_MV 3300.0
#define RESISTOR_400V 4422
#define RESISTOR_200V 2222
#define RESISTOR_3V 22

const float mvFactor1 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_400V / RESISTOR_3V;
const float mvFactor2 = ADC_FULL_MEASURE_MV / ADC_STEPS * RESISTOR_200V / RESISTOR_3V;
const float iFactor = 1;

float fM_VIN, fM_V225, fM_IHFL, fM_VOUT1, fM_VOUT2, fM_Temp;
float fM_V175, fM_IOUT, fM_IH1, fM_IH2, fM_IIN, fM_I175, fM_I225;

float ratioV225 = 150.0/400;
float ratioV175 = 250.0/400;

#define period  16364
static unsigned short compare_225 = period / 2;
static unsigned short compare_175 = period / 2;

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

void adjust_225_175()
{

	if (fabs(fM_VIN) < 10) {
		return; // low voltage at input
	}
	float target_175 = fM_VIN * ratioV175;
	float target_225 = fM_VIN * ratioV225;
	
	float diff_175 = (target_175 - fM_V175) / fM_VIN;
	float diff_225 = (target_225 - fM_V225) / fM_VIN;
	int diff;
	
	HRTIM_CompareCfgTypeDef compareCfg;

	// 225V TE1 PC8 test pin P1
	// 225 Timer E1 -> PC8  C_225 -> P1 -> U1(2) -> P23(1) -> Q1 L1 Q3 D3 
	diff = diff_225 * period / 8;
	int sign_or_0 = (0 < diff) - (diff < 0);
	diff =  (diff ? sign_or_0 : 0) + diff * max(compare_225, 96) / 40000;

	compareCfg.CompareValue = min(period - 96, max(95, compare_225 - diff));
	compare_225 = compareCfg.CompareValue; // remember previous value;

	if (compareCfg.CompareValue < 96) { // force inactive
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
	diff = diff_175 * period / 8;
	sign_or_0 = (0 < diff) - (diff < 0);
	diff =  (diff ? sign_or_0 : 0) + diff *  max(compare_175, 96) / 40000;

	compareCfg.CompareValue = min(period - 96, max(95, compare_175 + diff));
	compare_175 = compareCfg.CompareValue;

	if (compareCfg.CompareValue < 96) {
		stopped_175 = true;
		HAL_HRTIM_WaveformOutputStop(&hhrtim1,
			HRTIM_OUTPUT_TC1);
		HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_C,
			HRTIM_OUTPUT_TC1,
			HRTIM_OUTPUTLEVEL_INACTIVE);
	} else if (compareCfg.CompareValue >= period - 96) { // force active
		compareCfg.CompareValue = period - 1;
		HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
			HRTIM_TIMERINDEX_TIMER_E,
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



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adcHandle)
{// end of DMA
	if (adcHandle == &hadc2) {
		g_MeasurementNumber++;
		fM_VIN = g_ADCBuffer1[0] *mvFactor1;
		fM_V225 = g_ADCBuffer1[1] *mvFactor2;
		fM_IHFL = g_ADCBuffer1[2] *iFactor;
		fM_VOUT1 = g_ADCBuffer1[3] *mvFactor1;
		fM_VOUT2 = g_ADCBuffer1[4] *mvFactor1;
		fM_Temp = g_ADCBuffer1[5] * 1; 
		// ADC2
		fM_V175 = g_ADCBuffer2[0] *mvFactor2;
		fM_IOUT = g_ADCBuffer2[1] *mvFactor1; 
		fM_IH1 = g_ADCBuffer2[2] *iFactor; 
		fM_IH2 = g_ADCBuffer2[3] *iFactor;
		fM_IIN = g_ADCBuffer2[4] *iFactor; 
		fM_I175 = g_ADCBuffer2[5] *iFactor; 
		fM_I225 = g_ADCBuffer2[6] *iFactor; 
		adjust_225_175();
	}
	
}
