#include "Waveform.h"
#include "Command.h"
#include "tim.h"
#include <cmath>

extern TIM_HandleTypeDef htim15;

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

static int waveformIndex = WAVEFORM_SEGMENTS / 2;

void doNextWaveformSegment()
{
	static bool bIncreasing = true;
	if (bIncreasing) {
		waveformIndex++;
		if (waveformIndex >= WAVEFORM_SEGMENTS / 2) {
			bIncreasing = false;
		} 
	} else {
		waveformIndex--;
		if (waveformIndex <= 0) {
			bIncreasing = true;
		}
	}
#if 1
	int tim1Count = htim1.Instance->CNT;
	int tim1Period = htim1.Instance->ARR;
	int nSampleNumber = tim1Count % (tim1Period / 2);
	nSampleNumber = nSampleNumber * 4 *WAVEFORM_SEGMENTS / tim1Period;
	int nIndex = std::floor((std::abs(nSampleNumber * 2 - WAVEFORM_SEGMENTS * 2 + 1) + 1) / 4);
#else
	int nIndex = waveformIndex;
#endif
	setRt(iCosine[nIndex]);
}
