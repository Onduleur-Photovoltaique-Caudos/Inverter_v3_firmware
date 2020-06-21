#pragma once
#ifndef __WAVEFORM_H
#define __WAVEFORM_H

#ifdef __cplusplus
extern "C" {
#endif


	void doWaveformStep();
	float get3HD();

	void doAdjustPower();
	void setMaxPower(int newMax);
	int getMaxPower();
	bool getPowerLimitFlag();
	int getPower();
	void setFanSpeed(int speed);
	int getFanSpeed();
	void doAdjustFanSpeed();


#ifdef __cplusplus
}
#endif

#endif
