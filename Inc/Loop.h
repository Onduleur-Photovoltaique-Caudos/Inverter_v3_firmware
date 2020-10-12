#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
	void doLoop(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
	typedef enum {
		eMCOvercurrent,
		eMCEmergency,
		eMCQueueOverflow,
		eMCCurrentMeasurementsDump
	} eMessageCode;

	class Message {
	public:
		eMessageCode code;
		int val1;
		int val2;
		Message(){}
		Message(eMessageCode code, int val1 = 0, int val2 = 0)
			: code(code)
			, val1(val1)
			, val2(val2)
		{}
	};
	void queueMessage(Message message);

#endif



