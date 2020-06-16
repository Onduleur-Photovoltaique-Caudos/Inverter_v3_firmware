#pragma once
#ifndef __LM75_H
#define __LM75_H


// taken from github.com/jlz3008/lm75

#include <inttypes.h>
class TempI2C_LM75 {
	
public: 
	
	typedef enum { comparator_mode = 0, interrupt_mode } ThermostatMode;
	typedef enum { one_samples = 0, two_samples, four_samples, six_samples } ThermostatFaultTolerance;
	typedef enum { active_low = 0, active_high } OSPolarity;

	TempI2C_LM75(I2C_HandleTypeDef * hi2c, uint8_t i2c_addr);

	// Temperature and temperature ranges in degrees centigrade
	float acquireTemp(bool bIT = false);
	float getTemp();

	void setTHyst(float newTHyst);
	void setTOS(float newTOS);

	float getTHyst(void);
	float getTOS(void);

	ThermostatMode getThermostatMode();
	void setThermostatMode(ThermostatMode newMode);

	ThermostatFaultTolerance getThermostatFaultTolerance();
	void setThermostatFaultTolerance(ThermostatFaultTolerance newFaultTolerance);

	OSPolarity getOSPolarity();
	void setOSPolarity(OSPolarity newOSPolarity);

	bool getShutdown();
	void setShutdown(bool newShutdown);

	void storeTemp();

	static const uint8_t baseAddress = 0x48;
private:

	typedef enum  { temp_reg = 0, config_reg, THyst_reg, TOS_reg } LM75Register;

	typedef union _CfgRegister {
		struct {
			uint8_t shutdown : 1;
			uint8_t thermostat_mode : 1;
			uint8_t thermostat_output_polarity : 1;
			uint8_t thermostat_fault_tolerance : 2;
			uint8_t reserved : 3;
		} mbits;
		uint8_t mbyte;
	} CfgRegister;
	CfgRegister m_cfgRegister;
	typedef union _TempRegister {
		uint8_t mdata[2];
		unsigned short mTempX;
		short mTempS;
	} TempRegister;
	TempRegister m_tempRegister;
	float m_fTemp;
	TempRegister m_tempOS;
	TempRegister m_tempHyst;

	LM75Register previousReg = temp_reg;// default at initialization
	unsigned short getReg(LM75Register reg, uint8_t * ptrData, bool bIT);
	void setReg(LM75Register reg, unsigned newValue);

	uint16_t m_u16I2CAddr;
	I2C_HandleTypeDef * m_hi2c;
	
};


#endif
