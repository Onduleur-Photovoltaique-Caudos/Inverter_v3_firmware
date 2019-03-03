#pragma once

const int medianSize = 21;
#define MAX_MEDIANS 14


#ifdef __cplusplus
extern "C"
{
#endif
void do_initializeMedian();
#ifdef __cplusplus
}
#endif

unsigned short do_medianFilter(unsigned short n, unsigned short measure);


