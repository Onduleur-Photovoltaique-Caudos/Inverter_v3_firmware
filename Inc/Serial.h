#pragma once
#ifndef SERIAL_H
#define SERIAL_H

#include "usart.h"


#ifdef Error_Handler
#undef Error_Handler
#endif // Error_Handler

void cpp_Error_Handler(char * file, int line);

#define Error_Handler() cpp_Error_Handler(const_cast<char*>(__FILE__), __LINE__)


class SerialOutput {
public:
	SerialOutput(UART_HandleTypeDef * pHandle, char * driverBuffer, unsigned int size);
	bool puts(const char * str);
	bool putch(char c);
	bool putsNonBlocking(const char * str);
	void doOutputIT();

	static SerialOutput * channel_1;
	static SerialOutput * channel_2;
	static SerialOutput * channel_3;

	char * driverBuffer;
	unsigned int driverBufferSize;
	bool busy;
	UART_HandleTypeDef *pHandle;
};

static volatile int nLastError;

class SerialInput {
public:
	SerialInput(UART_HandleTypeDef * pHandle, char * buffer, unsigned int size);
	void initialize(SerialOutput * echoChannel=NULL) {
		if (HAL_UART_Receive_IT(pHandle, (uint8_t *)inputBuffer, 1) != HAL_OK) {
			nLastError = pHandle->ErrorCode;
			bEnabled = false;
		}
		bEnabled = true;
		echo = echoChannel;
	}
	void disable()
	{
		bEnabled = false;
	}
	void doInputIT();
	char * fgets(char * str, int size);
	char * fgetsNonBlocking(char * str, int size);

	static SerialInput * channel_1;
	static SerialInput * channel_2;
	static SerialInput * channel_3;

	bool bEnabled;
	SerialOutput * echo;
	char inputBuffer[2];
	bool eol;
	unsigned int overrun;
	unsigned int nChars;
	char * driverBuffer;
	char * driverBufferNextChar;
	unsigned int driverBufferSize;
	SerialInput * serialObject;
	UART_HandleTypeDef *pHandle;
};


#endif
