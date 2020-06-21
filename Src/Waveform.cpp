#include "Waveform.h"
#include "Command.h"
#include "Measure.h"
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

static int waveformIndex = WAVEFORM_SEGMENTS / 2 -1;
static bool bPositive=true;

static volatile bool bStopped = true;
float fVH3I, fVH3D, fVH3M;

volatile short maxPower;
volatile short nowPower = 100;
float fnowPower = 100;
volatile bool bLimitPower;

bool doNextWaveformSegment()
{
	bool bZeroCrossing = false;
	static bool bIncreasing = true;
	if (waveformIndex == (WAVEFORM_SEGMENTS / 6)) {
		 // measure harmonic 3 of input voltage here
		 if (bIncreasing){
			 fVH3I = fM_VIN;
		} else {
			fVH3D = fM_VIN;
		}
	}
	if (bIncreasing) {
		waveformIndex++;
		if (waveformIndex >= WAVEFORM_SEGMENTS / 2) {
			// measure peak of input voltage here
			fVH3M = fM_VIN;
			bIncreasing = false;
			bPositive = !bPositive;
			bZeroCrossing = true;
		}
	} else {
		if (waveformIndex >= WAVEFORM_SEGMENTS / 2) { // zeroCrossing here
		// compute harmonic distorsion to limit power
			if (isAC()) {
				  // AC sine waveform generation
					setOutputSlowSwitch(bPositive);
			} else {
				setOutputSlowSwitch(true);
			}
		}
		waveformIndex--;
		if (waveformIndex <= 0) {
			bIncreasing = true;
		}
	}

	int nIndex = waveformIndex;

	if (isAC()){  // AC sine waveform generation
		setRt(iCosine[nIndex]*getPower()/100);
	}
	return bZeroCrossing;
}


void doWaveformStep()
{
	//	doLedOn();
	if(bStopped)
	{
		bStopped = !isRun();
	} else {
		bool bZeroCrossing = doNextWaveformSegment();
		if (bZeroCrossing) {
			doTemperatureAcquisitionStep();
		}
		if (bZeroCrossing && !isRun()) {
			// we prefer to stop at zero crossing
			bStopped = true;
		}
	}
	//	doLedOff();
}

void setMaxPower(int newMax)
{
	if (newMax == 0) {
		bLimitPower = false;
		return;
	} 
	bLimitPower = true;
	maxPower = newMax;
}
int getMaxPower()
{
	return maxPower;
}
bool getPowerLimitFlag(){
	return bLimitPower;
}
int getPower()
{
	return nowPower;
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
	fnowPower = std::max(100.0f, fnowPower*(1 + adjustment / ADJUSTMENT_TIME_CONSTANT));
	nowPower = fnowPower;
	if (bLimitPower && nowPower > maxPower) {
		nowPower = maxPower;
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

void doAdjustFanSpeed()
{
	if (bFanSpeedIsDefined){
		return;
	}
	float powerIn;
	powerIn = getIIN()*getVIN() / 1000;
	fanSpeed = powerIn / 3000;
	setFanPWM(fanSpeed);
}