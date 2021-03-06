#pragma once
#ifndef __WAVEFORM_H
#define __WAVEFORM_H

#ifdef __cplusplus
extern "C" {
#endif
	void doSecondHalfStep();
	void executeSetRt();
	void doResetHalfStep();
	void doStartAC();
	void doResetWaveform();
	bool doWaveformStep();
	float get3HD();

	void doAdjustPower();
	void setMaxPower(int newMax);
	int getMaxPower();
	bool getPowerLimitFlag();
	int getPowerLimit();
	void setFanSpeed(int speed);
	int getFanSpeed();
	void doAdjustFanSpeed();
	float getPowerIn();


#ifdef __cplusplus
}
#endif

#endif
