#include "MedianFilter.h"

class MedianEntry {
public:
	unsigned short number;
	unsigned short measure;

	MedianEntry(unsigned short number, unsigned short measure)
		: number(number)
		, measure(measure)
	{}
	;
	MedianEntry(void)
		: number(0)
		, measure(0)
	{}
	;
} ;


//volatile unsigned int medianTime = 0;
MedianEntry medianEntries[MAX_MEDIANS][medianSize];
unsigned int oldestEntry[MAX_MEDIANS];
unsigned int newestEntry[MAX_MEDIANS];

void do_initializeMedian()
{
	for (int n = 0; n < MAX_MEDIANS; n++) {
		unsigned int i;
		for (i = 0; i < medianSize; i++) {
			medianEntries[n][i] = MedianEntry(i, 10);
		}
		oldestEntry[n] = 0;
		newestEntry[n] = i - 1;
	}
}

unsigned short do_medianFilter(unsigned short n, unsigned short measure)
{
	int i;
	unsigned short value;
	bool bFound = false;
	// look for oldest entry and squash it from there to the end
	for (i = 0; i < medianSize; i++) {
		if (bFound) {
			medianEntries[n][i - 1] = medianEntries[n][i];
		}
		if (medianEntries[n][i].number == oldestEntry[n]) {
			bFound = true;
		}
	}
	newestEntry[n]++;
	oldestEntry[n]++;
	// find where to insert the new entry squashing the last entry
	if (measure > medianEntries[n][medianSize - 2].measure) {
		// the largest entry is
		medianEntries[n][medianSize - 1] = MedianEntry(newestEntry[n], measure);
	} else {
	// make space for the new entry by shifting towards the end
		bFound = false;
		for (i = medianSize - 2; i > 0; i--) {
			medianEntries[n][i + 1] = medianEntries[n][i];
			if (measure < medianEntries[n][i].measure &&
				measure > medianEntries[n][i - 1].measure) {
					medianEntries[n][i] = MedianEntry(newestEntry[n], measure);
				goto done;
			}
		}
		medianEntries[n][1] = medianEntries[n][0];
		medianEntries[n][0] = MedianEntry(newestEntry[n], measure);
	}
done:
	value = medianEntries[n][medianSize / 2].measure;
	return value;
}
