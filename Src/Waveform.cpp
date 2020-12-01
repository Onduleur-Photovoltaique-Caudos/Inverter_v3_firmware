#include "Waveform.h"
#include "Command.h"
#include "Measure.h"
#include "gpio.h"
#include "tim.h"
#include <cmath>
#include "Temperature.h"
#include <algorithm> 

// tim15 period 15, segments 32
#define WAVEFORM_SEGMENTS 32
/* sine wave synthesis:
 * WAVEFORM_SEGMENTS is noted N below is the number of segments in half a period
 * the clock interrupts 2N times by half period
 * the table has N/2+1 entries and is generated in excel with the formula
 * =ROUND(COS(i * PI / N)*92,0)  where i is the table index 92 and 4 being min and max amplitudes
 */
int iCosine[] = { 92,
92,
90,
88,
85,
81,
77,
71,
65,
58,
51,
43,
35,
27,
18,
9,
0
};

static int zeroCrossingWaveformIndex = WAVEFORM_SEGMENTS / 2 -1;
static int waveformIndex = 0;
static bool bPositive=true;

static volatile bool bStopped = true;
float fVH3I, fVH3D, fVH3M;

volatile bool bLimitPower;
volatile short powerLimit;
volatile short nowPowerAdjust = 100;
float fnowPowerAdjust = 100;

bool bIncreasing = true;
bool doNextWaveformSegment()
{
	bool bZeroCrossing = false;

	if (bIncreasing) {
		waveformIndex++;
	} else {
		waveformIndex--;
	}

	if (waveformIndex == (WAVEFORM_SEGMENTS / 6)) {
		 // measure harmonic 3 of input voltage here
		 if (bIncreasing){
			 fVH3I = fM_VIN;
		} else {
			fVH3D = fM_VIN;
		}
	}
	if (bIncreasing) {
		if (waveformIndex >= WAVEFORM_SEGMENTS / 2) {
			bIncreasing = false;
			bPositive = !bPositive;
			bZeroCrossing = true;
		}
	} else {
		if (waveformIndex <= 0) {
			bIncreasing = true;
			// measure peak of input voltage here
			fVH3M = fM_VIN;
		}
	}

	int nIndex = waveformIndex;

	if (getACState()) {
		  // AC sine waveform generation
		setRt(iCosine[nIndex]*getPowerLimit()/100);
	}
	return bZeroCrossing;
}

static bool bUpDown;

void doResetUpDown()
{
	bUpDown = false;
}

void doWaveformStep()
{

	if(getACState())
	{
		bool bZeroCrossing = doNextWaveformSegment();
		if (bZeroCrossing) {
			bUpDown = !bUpDown;
			if (bUpDown) {
				doLedOn();
				//doSwitchUp();
			} else {
				doLedOn();
				//doSwitchDown();
			}
			//doTemperatureAcquisitionStep();
		}
		if (bZeroCrossing && !isACWanted()) {
			// we prefer to stop at zero crossing
			setACState(false);
		}
	}
	//doPlanSwitch();
}

void doResetWaveform()
{ //timer 1 cyclic processing
	waveformIndex = 0;
	bPositive = true;
	bIncreasing = true;
	doRestartTim2Tim3();
}

void doStartAC()
{ // start from idle at middle of waveform (zero crossing)
	waveformIndex = zeroCrossingWaveformIndex;
	doResetUpDown();
	bPositive = true;
	bIncreasing = false;
	setACState(true);
	setACWanted(true);
	doStartTim1Tim2Tim3AtZeroCrossing();
	//doPsensePulse();
}

void setMaxPower(int newMax)
{
	if (newMax == 0) {
		bLimitPower = false;
		return;
	} 
	bLimitPower = true;
	powerLimit = newMax;
}
int getMaxPower()
{
	return powerLimit;
}
bool getPowerLimitFlag(){
	return bLimitPower;
}
int getPowerLimit()
{
	return nowPowerAdjust;
}

float harmonicDistortion;
#define ADJUSTMENT_TIME_CONSTANT 10
#define TARGET_HD 0.05

float get3HD(){
	return harmonicDistortion;
}
static float compute3HD() // third harmonic distortion
{
	float hd = fabs((fVH3I + fVH3D - fVH3M) / fVH3M);
	return hd;
}


static void adjustPower(float adjustment)
{
	fnowPowerAdjust = std::max(100.0f, fnowPowerAdjust*(1 + adjustment / ADJUSTMENT_TIME_CONSTANT));
	nowPowerAdjust = fnowPowerAdjust;
	if (bLimitPower && nowPowerAdjust > powerLimit) {
		nowPowerAdjust = powerLimit;
	}
}

void doAdjustPower(){	
	harmonicDistortion=compute3HD();
	adjustPower(TARGET_HD - harmonicDistortion);
}

static bool bFanSpeedIsDefined;
static int fanSpeed = 20;

int getFanSpeed(){
	return fanSpeed;
}
void setFanSpeed(int newSpeed)
{
	if (newSpeed == 0) {
		bFanSpeedIsDefined = false;
	} else {
		bFanSpeedIsDefined = true;
		fanSpeed = newSpeed;
		setFanPWM(newSpeed);
	}
}

float powerIn;
void doAdjustFanSpeed()
{
	static float fFanSpeed = 20.0f;
	if (bFanSpeedIsDefined){
		return;
	}
	powerIn = getIIN()*getVIN() / 1000;
	fanSpeed = 100* powerIn / 3000;
	// do IIR first order filter
	fFanSpeed = 0.99f * fFanSpeed + 0.01f * fanSpeed;
	setFanPWM(fFanSpeed);
}

float getPowerIn(){
	return powerIn;
}
