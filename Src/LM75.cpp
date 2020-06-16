extern "C"
{
#include "main.h"
}

#include "i2c.h"
#include "stm32f3xx_hal_i2c.h"
#include "LM75.h"

//-------------------------------------------------------------------------------

TempI2C_LM75::TempI2C_LM75(I2C_HandleTypeDef * hi2c,uint8_t i2c_addr)
{
	// on STM32, I2C addresses have to be shifted 1 bit left to allow for hardware insertion of the r/w bit as the MSb
	uint8_t _i2cAddr = (i2c_addr << 1);
   
	int status;
	//status = HAL_I2C_Master_Receive(hi2c, (uint16_t)_i2cAddr<<1, &_shadow, 1, 10);
	status = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)_i2cAddr, 3, 1000); 
	m_tempRegister.mTempX = 0;
	m_fTemp = 0.0F;
	if (status != HAL_OK) {
		m_u16I2CAddr = 0; // does not answer
	} else {
		m_u16I2CAddr = _i2cAddr;
		m_hi2c = hi2c;
		bool verifyShutdown;
		setShutdown(0);
		verifyShutdown=getShutdown();
		acquireTemp();
	}
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTemp()
{
	return m_fTemp;
}
float TempI2C_LM75::acquireTemp(bool bIT)
{
	int status;
	if (bIT) {
		m_tempRegister.mTempX = 0;
	}
	status = getReg(temp_reg,&m_tempRegister.mdata[0],bIT);
	if (!bIT){
		storeTemp();
		return getTemp();
	} 
	return 0.0F;
}
void TempI2C_LM75::storeTemp()
{
	uint8_t data; // we have to swap
	data = m_tempRegister.mdata[0];
	m_tempRegister.mdata[0] = m_tempRegister.mdata[1];
	m_tempRegister.mdata[1] = data;
	m_fTemp = m_tempRegister.mTempS / 256.0F;
}
//-------------------------------------------------------------------------------
TempI2C_LM75 * pSensorIT;
unsigned short TempI2C_LM75::getReg(LM75Register reg, uint8_t * ptrData, bool bIT)
{
	unsigned short retVal = 0;
	bool regAlreadySet = (reg == previousReg);

	if (m_u16I2CAddr) {
		int status;
		if (reg == config_reg) {
			if (regAlreadySet) {
				if (bIT) {
					pSensorIT = this;
					status = HAL_I2C_Master_Receive_DMA(m_hi2c, m_u16I2CAddr, ptrData, 1);
				} else {
					status = HAL_I2C_Master_Receive(m_hi2c, m_u16I2CAddr, ptrData, 1, 1000);
				}
			} else {
				status = HAL_I2C_Mem_Read(m_hi2c, m_u16I2CAddr, reg, I2C_MEMADD_SIZE_8BIT, ptrData, 1, 1000);
			}
			retVal = status;  
		} else {
			if (regAlreadySet) {
				if (bIT) {
					pSensorIT = this;
					status = HAL_I2C_Master_Receive_DMA(m_hi2c, m_u16I2CAddr, ptrData, 2);
				} else {
					status = HAL_I2C_Master_Receive(m_hi2c, m_u16I2CAddr, ptrData, 2, 1000);
				}
			} else {
				status = HAL_I2C_Mem_Read(m_hi2c, m_u16I2CAddr, reg, I2C_MEMADD_SIZE_8BIT, ptrData, 2, 1000);
			}
			retVal = status;  
		}
		previousReg = reg;
	}
	return retVal;
}
//-------------------------------------------------------------------------------
void TempI2C_LM75::setReg(LM75Register reg, unsigned newValue)
{
	int status = HAL_ERROR;

	if (m_u16I2CAddr) {
		// Only write HIGH the values of the ports that have been initialised as
		// outputs updating the output shadow of the device
		uint8_t data[3];
		short length;
		
		data[0] = newValue & 0xFF;
		if (reg == config_reg) {
			data[0] = newValue;

			status = HAL_I2C_Mem_Write(m_hi2c, m_u16I2CAddr, reg, I2C_MEMADD_SIZE_8BIT, &data[0], 1, 1000);
		} else {
			length = 1;
			data[1] = (newValue & 0xFF00) >> 8;

			status = HAL_I2C_Mem_Write(m_hi2c, m_u16I2CAddr, reg, I2C_MEMADD_SIZE_8BIT, &data[0], 2, 1000);
		}
		previousReg = reg;
	}
	//return ((status == HAL_OK)); // HAL_OK is 0 as well
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setTHyst(float newTHyst)
{
	setReg(THyst_reg, int(newTHyst * 256));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setTOS(float newTOS)
{
	setReg(TOS_reg, int(newTOS * 256));
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTHyst(void)
{
	getReg(THyst_reg, & m_tempHyst.mdata[0], false);
	return (m_tempHyst.mTempS / 256.0F);
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTOS(void)
{
	getReg(THyst_reg, & m_tempOS.mdata[0], false);
	return (m_tempOS.mTempS / 256.0F);
}

//-------------------------------------------------------------------------------
TempI2C_LM75::ThermostatMode TempI2C_LM75::getThermostatMode()
{
	getReg(config_reg,&m_cfgRegister.mbyte,false);

	return (ThermostatMode(m_cfgRegister.mbits.thermostat_mode));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setThermostatMode(TempI2C_LM75::ThermostatMode newMode)
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	m_cfgRegister.mbits.thermostat_mode = newMode;
	m_cfgRegister.mbits.reserved = 0;
	setReg(config_reg, unsigned(m_cfgRegister.mbyte));
}

//-------------------------------------------------------------------------------
TempI2C_LM75::ThermostatFaultTolerance TempI2C_LM75::getThermostatFaultTolerance()
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	return (ThermostatFaultTolerance(m_cfgRegister.mbits.thermostat_fault_tolerance));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setThermostatFaultTolerance(ThermostatFaultTolerance newFaultTolerance)
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	m_cfgRegister.mbits.thermostat_fault_tolerance = newFaultTolerance;
	m_cfgRegister.mbits.reserved = 0;
	setReg(config_reg, unsigned(m_cfgRegister.mbyte));
}


bool TempI2C_LM75::getShutdown()
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	return (m_cfgRegister.mbits.shutdown);
}
void TempI2C_LM75::setShutdown(bool newShutdown)
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	m_cfgRegister.mbits.shutdown = newShutdown;
	m_cfgRegister.mbits.reserved = 0;
	setReg(config_reg, unsigned(m_cfgRegister.mbyte));
}
//-------------------------------------------------------------------------------
TempI2C_LM75::OSPolarity TempI2C_LM75::getOSPolarity()
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	return (OSPolarity(m_cfgRegister.mbits.thermostat_output_polarity));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setOSPolarity(OSPolarity newOSPolarity)
{
	getReg(config_reg, &m_cfgRegister.mbyte, false);

	m_cfgRegister.mbits.thermostat_output_polarity = newOSPolarity;
	m_cfgRegister.mbits.reserved = 0;
	setReg(config_reg, unsigned(m_cfgRegister.mbyte));
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (pSensorIT) {
		pSensorIT->storeTemp();
	}
}