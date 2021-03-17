extern "C" {
#include "main.h"
}

#include "Serial.h"
#include "gpio.h"
#include <string.h>

SerialInput * SerialInput::channel_1 = NULL;
SerialInput * SerialInput::channel_2 = NULL;
SerialOutput * SerialOutput::channel_1 = NULL;
SerialOutput * SerialOutput::channel_2 = NULL;

void SerialErrorHandler(int nReason,int nSecondary=0)
{
	doDisableSwitches();
	doLedOff();
	while (true) {
		delay_us_DWT(2000000);
		for (int i=0;i< nReason;i++){
			doLedOn();
			delay_us_DWT(200000);
			doLedOff();
			delay_us_DWT(500000);
			}
		delay_us_DWT(2000000);
		for (int i = 0; i < nSecondary; i++) {
			doLedOn();
			delay_us_DWT(200000);
			doLedOff();
			delay_us_DWT(500000);
		}
		delay_us_DWT(2000000);
	}
	Error_Handler();
}
void cpp_Error_Handler(char * x,int line){
	Error_Handler();
}

SerialInput::SerialInput(UART_HandleTypeDef * pHandle, char * buffer, unsigned int size) : pHandle(pHandle), driverBuffer (buffer), driverBufferSize (size)
{

	driverBufferNextChar = buffer;
	nChars = 0;
	eol = false;
	overrun = 0;
	inputBuffer[0] = 0;
	inputBuffer[1] = 0;

	if (pHandle == &huart2) {
		SerialInput::channel_2 = this;
	} else {
		SerialErrorHandler(1);
	}
}

// append terminating 0, returns the newline and clip to size characters
char * SerialInput::fgets(char * str, int size)
{
	while (!eol && ((driverBufferNextChar - driverBuffer) < size)) {
		HAL_Delay(1);
	}
	return fgetsNonBlocking(str, size);
}

char *  SerialInput::fgetsNonBlocking(char * str, int size)
{
	if (! bEnabled){
		return NULL;
	}
	while (!eol && ((driverBufferNextChar - driverBuffer) < size)) {
		return NULL;
	}
	int sizeUseful;
	if ((driverBufferNextChar - driverBuffer) >= size) {
		sizeUseful = size - 1;
		strncpy(str, driverBuffer, sizeUseful);
		str[sizeUseful] = 0;
		goto cleanup;
	} else {
		sizeUseful = driverBufferNextChar - driverBuffer;
		strncpy(str, driverBuffer, sizeUseful);
		str[sizeUseful] = 0;
	}
cleanup:
	driverBufferNextChar = driverBuffer;
	nChars = 0;
	overrun = 0;
	eol = false;
	return str;
}

void SerialInput::doInputIT(void)
{
	if (!bEnabled) {
		return;
	}
	char c = inputBuffer[0];
	if (echo) {
		echo->putch(c);
		if (c == '\r') {
			echo->putch('\n');
		}
	}
	if ((driverBufferNextChar - driverBuffer) >= driverBufferSize) {
		overrun++;
	} else {
		if (c == '\n') {
			eol = true;
		}
		if (c == '\r') {
			c = '\n';
			eol = true;
		}
		if (c == 127 || c == 8) {
			if (nChars > 0) {
				driverBufferNextChar--;
				nChars--;
			}
		} else {
			*driverBufferNextChar++ = c;
			nChars++;
		}
	}
	HAL_StatusTypeDef status = HAL_UART_Receive_IT(pHandle, (uint8_t *)inputBuffer, 1);
	if (status != HAL_OK) {
		//bEnabled = false;
		//SerialErrorHandler(2);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	SerialInput * serialObject;
	if (UartHandle == &huart2) {
		serialObject  = SerialInput::channel_2;
	} else {
		SerialErrorHandler(3);
	}
	serialObject->doInputIT();
}

SerialOutput::SerialOutput(UART_HandleTypeDef * pHandle, char * driverBuffer, unsigned int size)
								:pHandle(pHandle),driverBuffer(driverBuffer),driverBufferSize(size)
{
	busy = false;
	if(pHandle == &huart2){
		SerialOutput::channel_2 = this;
	} else {
		SerialErrorHandler(4);
	}
}

bool SerialOutput::puts(const char * str){
	int statusTransmit;
	busy = true;
	if (strlen(str) >= driverBufferSize) {
	}
	statusTransmit = HAL_UART_Transmit(pHandle, (uint8_t*)str, strlen(str), 1000);
	busy = false;
	return statusTransmit;
}

bool SerialOutput::putch(char c)
{
	int statusTransmit;
	busy = true;
	statusTransmit = HAL_UART_Transmit(pHandle, (uint8_t*)&c, 1, 1000);
	busy = false;
	return statusTransmit;
}


bool SerialOutput::putsNonBlocking(const char * str)
{
	int statusTransmit;
	if (busy) {
		return false;
	}
	busy = true;
	int len = strlen(str);
//	if (strlen(str) >= driverBufferSize) {
//	}

	statusTransmit = HAL_UART_Transmit_IT(pHandle, (uint8_t*) str, len);
	return statusTransmit;
}

void SerialOutput::doOutputIT(){
	busy = false;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	SerialOutput * serialObject;
	if (UartHandle == &huart2) {
		serialObject  = SerialOutput::channel_2;
	} else {
		SerialErrorHandler(5);
	}
	serialObject->doOutputIT();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
//  HAL_UART_ERROR_NONE      = 0x00,    /*!< No error            */
//	HAL_UART_ERROR_PE        = 0x01,    /*!< Parity error        */
//	HAL_UART_ERROR_NE        = 0x02,    /*!< Noise error  happens if not connected       */
//	HAL_UART_ERROR_FE        = 0x04,    /*!< frame error  happens if not connected       */
//	HAL_UART_ERROR_ORE       = 0x08,    /*!< Overrun error       */
//	HAL_UART_ERROR_DMA       = 0x10,    /*!< DMA transfer error  */
//	HAL_UART_ERROR_BUSY      = 0x20     /*!< Busy Error          */
	nLastError = huart->ErrorCode;
	if (huart == &huart2) {
		SerialInput * serialObject;
		serialObject  = SerialInput::channel_2;
#define ERROR_TEXT "\r\nSerial input error\r\n"
		HAL_UART_Transmit(huart, (uint8_t*)ERROR_TEXT, strlen(ERROR_TEXT), 1000);
		//serialObject->disable();
	} else {
		SerialErrorHandler(6, nLastError);
	}
}
