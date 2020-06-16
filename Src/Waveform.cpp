#include "Waveform.h"
#include "Command.h"
#include "Measure.h"
#include "tim.h"
#include <cmath>
#include "Temperature.h"


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

bool doNextWaveformSegment()
{
	bool bZeroCrossing = false;
	static bool bIncreasing = true;
	if (waveformIndex == (WAVEFORM_SEGMENTS / 4) + 1) {
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
		setRt(iCosine[nIndex]);
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
