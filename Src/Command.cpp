extern "C"
{
#include "main.h"
}
#include "stm32f3xx_hal.h"
#include "Command.h"
#include "Loop.h"
#include "Waveform.h"
#include "gpio.h"
#include <cstring>
#include "Measure.h"
#include "Temperature.h"
#include "hrtim.h"
#include "tim.h"
#include "stdlib.h"


#include "Serial.h"

#define SERIAL_BUFFER_SIZE 50
static char bufferOutConsole[SERIAL_BUFFER_SIZE];
static char bufferInConsole[SERIAL_BUFFER_SIZE];
static SerialOutput SerialOutToConsole(&huart2, bufferOutConsole, SERIAL_BUFFER_SIZE);	 // via USB
static SerialInput SerialInFromConsole(&huart2, bufferInConsole, SERIAL_BUFFER_SIZE);
static SerialOutput* pSerialOutToConsole;
static SerialInput* pSerialInFromConsole;

typedef enum _runState {
	eStop,
	eStopEmergency,
	eDelayed,
	eRun
} RunState;

volatile RunState runState;

volatile bool stateACWanted;    // want to produce AC waveform

volatile bool stateAC;  // producing AC waveform
volatile t_breakerState breakerState = eNormal;
volatile unsigned long long runDelayTimerStartTick;

void initializeCommand()
{
	setRt(10);

	setT1(20);
	setZ1(180);
	setD1(-20);
	setV1(170);

	setT2(20);
	setZ2(180);
	setD2(-20);
	setV2(230);

	pSerialOutToConsole = &SerialOutToConsole;
	pSerialInFromConsole = &SerialInFromConsole;
	pSerialInFromConsole->initialize(pSerialOutToConsole);
	pSerialOutToConsole->puts("\r\nReady\r\n");

	doRunJustBooted();
}

void logMessage(const char * message){
	pSerialOutToConsole->puts(message);
}

void displayRecordedMeasurements()
{
	int recordCount = getMeasurementRecordCount();
	for (int i = 0; i < recordCount; i++) {
		pSerialOutToConsole->puts(my_itoa(getMeasurementNextRecord() * 10)); pSerialOutToConsole->puts("\t");
	}
	pSerialOutToConsole->puts("\r\n");
}

t_breakerState getBreakerState()
{
	return breakerState;
}
void setBreaker(t_breakerState newState, float f_IIN, float max_IIN, float f_IOUT, float max_IOUT)
{
	breakerState = newState;
	switch(newState) {
	case eNormal:
		break;
	case eOver:
		queueMessage(Message(eMCOvercurrent,f_IIN,f_IOUT));
		break;
	case eEmergency:
		queueMessage(Message(eMCEmergency, f_IIN, f_IOUT));
		break;
	}
}

void setBreakerRearm(int value){
	if (value != 0) {// command ar=1 : rearm
		breakerState = eNormal;
		doRunJustBooted();
	} else { // command ar=  break
		setBreaker(eEmergency, 100, 99, 100, 99);
	}
}


void initializeRunTimer()
{
	runDelayTimerStartTick = getMeasureCount();
}
#define RUN_DELAY_MS 2000
bool runDelayTimerFinished()
{
	unsigned long long currentTick = getMeasureCount();
	if (breakerState!= eNormal){
		return false;
	}
	return currentTick > (runDelayTimerStartTick + RUN_DELAY_MS * 8);
}

bool setACWanted(int newACWanted)
{
	bool oldACWanted = stateACWanted;
	stateACWanted = newACWanted != 0;
	return oldACWanted;
}

bool setACState(int newACState)
{
	bool oldState = stateAC;
	stateAC = newACState != 0;
	return oldState;
}

bool getACState()
{
	return stateAC;
}

static bool lastACstate;  // we remember last AC, so we can 

bool isACWanted()
{
	return stateACWanted;
}

void doAC(int newState)
{
	if (newState) {
		doStartAC();
	} else {
		setACWanted(false);
	}
}

	bool isRun()
{
//debug mode for lab tests: FORCE_RUN:1
#define FORCE_RUN 1
#if FORCE_RUN
	return true;
#else
	return runState == eRun;
#endif
}
bool isStop()
{
	return runState == eStop;
}

bool isEmergencyStop()
{ 
	return runState == eStop || runState == eStopEmergency;
}

void doRunJustBooted()
{
	runState = eStop;
}
void doRunLowVoltage()
{
	runState = eStop;
}

void doRunNormalVoltage()
{
	if (runState == eStop) {
		runState = eDelayed;
		initializeRunTimer();
	} else if (runState == eDelayed){
		if (runDelayTimerFinished()) {
			runState = eRun;
		}
	}
}

 typedef enum _command_t {
	none,
	st,
	rt,
	tt,
	ac,
	ar,
	sm,
	po,
	fa,
	z1,
	z2,
	d1, 
	d2, 
	t1,
	t2,
	v1,
	v2
 } command_t;

const command_t firstCompositeCommand = z1;
const command_t commandArray[] =  { 
	none,
	st,
	rt,
	tt,
	ac,
	ar, 
	sm,
	po,
	fa, // end of simple commands
	z1, // start of composite commands
	z2,
	d1, 
	d2, 
	t1,
	t2,
	v1,
	v2
};
const char * simpleCommandListStr[] = {
	"no", 
	"st", 
	"rt", 
	"tt", 
	"ac", 
	"ar", 
	"sm", 
	"po",
	"fa"
	};
command_t command;
/* Commands 2 alphanumerical, = sign, integer 
st=  // display status
tt=  // display temperatures
ac = {0,1} // generate sinewave
ar = {0,1} // 0=breaker test  1=rearm breaker
sm= report measurements
po=n    max power
rt=n    ratio
z1=n	zvs pulse width ns for low side (going up)
z2=n	zvs pulse width in ns for hi side (going down)
d1=n	low side pulse delay  (negative means overlap)
d2=n	hi side pulse delay 
t1=n	low side trailing edge delay (negative means shorter trailing edge)
t2=n	hi side trailing edge delay
v1=n	low side zvs voltage (225 V)
v2=n	hi size zvs voltage (175 V)
*/
class commandAndValue{
public:
	commandAndValue(command_t command, int value)
		: command(command)
		, value(value)
	{
	}
	command_t command;
	int value;
};


// 250 ps per count => 4 per ns
#define COUNT_PER_NS 4

int _rt;
int _base;
int _countZ1 = 120 * COUNT_PER_NS;  // see initializeCommand
int _countZ2 = 130 * COUNT_PER_NS;   // see initializeCommand
int _countD1 = -20 * COUNT_PER_NS;
int _countD2 = 0 * COUNT_PER_NS;
int _countT1 = 0 * COUNT_PER_NS;
int _countT2 = 0 * COUNT_PER_NS;

#define PERIOD_SWITCH 16000
#define PER_CENT 100

void sendSerial(const char* message){
	pSerialOutToConsole->puts(message);
}
void statusDisplay(bool bHeader){
	if (bHeader) {
		pSerialOutToConsole->puts("    rt\t    z1\t    d1\t    r1\t    v1\t     z2\t    d2\t    r2\t    v2\n\r");
	}
	pSerialOutToConsole->puts(my_itoa(_rt));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countZ1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countD1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_base / COUNT_PER_NS));
	//pSerialOutToConsole->puts("\t");
	//pSerialOutToConsole->puts(my_itoa(_countT1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(getRatioV225()*400));
	pSerialOutToConsole->puts("\t"); 
	pSerialOutToConsole->puts(my_itoa(_countZ2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countD2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa((PERIOD_SWITCH - _base) / COUNT_PER_NS));
	//pSerialOutToConsole->puts("\t");
	//pSerialOutToConsole->puts(my_itoa(_countT2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(getRatioV175() * 400));
	pSerialOutToConsole->puts("\n\r");
}

void temperaturesDisplay(int reinit)
{
	for (int i=0; i < TEMPERATURE_SENSOR_COUNT ;i++){
		if (reinit != 0) {
			pSerialOutToConsole->puts(my_itoa(getTemp(i)));
		} else {
			pSerialOutToConsole->puts(my_itoa(acquireTemp(i)));
		}
		if (i < TEMPERATURE_SENSOR_COUNT - 1) {
			pSerialOutToConsole->puts("\t");
		} else {
			pSerialOutToConsole->puts("\n\r");
		}
	}
}

void measurementsDisplay(bool bHeader, int paragraph)
{
	switch (paragraph) {
	case 0:
	if (bHeader) {
		pSerialOutToConsole->puts("  Vin\t   Iin\t  Iout\t I175\t I225\tVout1\tVout2\tTemp\tV175\tV225\n\r");
	}
	pSerialOutToConsole->puts(my_itoa(fM_VIN / 1000,1000)); 			pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_IIN * 1000, 100000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_IOUT * 1000, 100000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_I175 * 1000, 10000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_I225 * 1000, 10000));
	pSerialOutToConsole->puts(my_itoa(fM_VOUT1 / 1000,1000)) ;		pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_VOUT2 / 1000,1000)) ;		pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_Temp,1000));				pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_V175 / 1000,1000)) ;		pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_V225 / 1000, 1000)); pSerialOutToConsole->puts("\t");
			break;
	case 1:
		if (bHeader) {
			pSerialOutToConsole->puts("Factor\tIhfl\tIh1\tIh2\t     \t    \t    \t    \n\r");
		}
	pSerialOutToConsole->puts(my_itoa(mvCorrectionFactor * 1000, 10000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_IHFL  * 1000, 100000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_IH1 * 1000, 100000)); pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(fM_IH2 * 1000, 100000)); pSerialOutToConsole->puts("\t");
		break;
	case 2:
		if (bHeader) {
			pSerialOutToConsole->puts("PowF\tPowL\tPow\tFan\tTHD\tIh2\t     \t    \t    \t    \n\r");
		}
		pSerialOutToConsole->puts(my_itoa(getPowerLimitFlag ()? 999 : getMaxPower(), 1000)); pSerialOutToConsole->puts("\t");
		pSerialOutToConsole->puts(my_itoa(getPowerLimit(),100)); pSerialOutToConsole->puts("\t");
		pSerialOutToConsole->puts(my_itoa(getPowerIn(), 10000)); pSerialOutToConsole->puts("\t");
		pSerialOutToConsole->puts(my_itoa(getFanSpeed(), 100)); pSerialOutToConsole->puts("\t");
		pSerialOutToConsole->puts(my_itoa(get3HD()*100,100)); pSerialOutToConsole->puts("\t");

	}
	pSerialOutToConsole->puts("\n\r");
}

void doZ1(void){
	setCompareA1(_countZ1);
	setCompareA2(_countZ1 + _countD1);
}
void setZ1(int val){
	int count = val * COUNT_PER_NS;
	_countZ1 = count;
	doZ1();
	doUpdateTimA();
}
void doZ2(void){
	setCompareB1(_countZ2);
	setCompareB2(_countZ2 + _countD2);
}
void setZ2(int val)
{
	int count = val * COUNT_PER_NS;
	_countZ2 = count;
	doZ2();
	doUpdateTimB();
}
void doD1(void){
	setCompareA2(_countZ1 + _countD1);
}
void setD1(int val)
{
	int count = val * COUNT_PER_NS;
	_countD1 = count;
	doD1();
	doUpdateTimA();
}
void doD2(void){
	setCompareB2(_countZ2 + _countD2);
}
void setD2(int val)
{
	int count = val * COUNT_PER_NS;
	_countD2 = count;
	doD2();
	doUpdateTimB();
}
void doT1(){
	setCompareB3(_base + _countT1);
}
void setT1(int val)
{
	int count = val * COUNT_PER_NS;
	_countT1 = count;
	doT1();
	doUpdateTimB();
}
void doT2(void){
	setCompareA3(PERIOD_SWITCH - _base + _countT2);
}
void setT2(int val)
{
	int count = val * COUNT_PER_NS;
	_countT2 = count;
	doT2();
	doUpdateTimA();
}

void doEmergencyStop()
{
	setRt(0);
}

volatile bool mustUpdateMAB;

void setRt(int valRt)
{
	if (valRt > 92)
		valRt = 100;
	if (valRt < 9)
		valRt = 0;
	int valueA2;
	int valueB2;
	int base = (PERIOD_SWITCH * valRt) / PER_CENT;
	int baseReduction = 0;
	_base = base;
	if (_base > PERIOD_SWITCH - 0x20) {
		_base = PERIOD_SWITCH - 0x20; // otherwise CompareMaster1 does not trigger TIMB
		baseReduction = 0x20 + base - PERIOD_SWITCH; 
	}
	int valueA3 = PERIOD_SWITCH - _base + _countT1;
	if (valueA3 == PERIOD_SWITCH) {
		valueA3 = PERIOD_SWITCH + 1; // make sure lower switch is a permanent 1
	}
	if (valRt >= _rt && valRt == 92 && _rt != 92) {
//		doLedToggle();
	}
	if (_base < _countZ2 + _countD2) { // going to zero
		valueB2 = _base;
		setCompareB2(valueB2);
		setOutputB2(0);
		setCompareA2(0x20); // use minimum value so we have a set
		setCompareA3(PERIOD_SWITCH + 0x20);
		// supporess the ZVS pulses
		doStopZVS(true);
		doOutputSetSourceA1None(); // lower switch
		setOutputA1(0); 
		doOutputSetSourceB1None(); //upper switch
		setOutputB1(0); 
	} else if (valueA3 < _countZ1 + _countD1) { // going to 100
		setCompareA2(0); // less than minimum, so we do not have the set
		setCompareA3(0x20); // reset immediately
		setCompareB2(0x20); // set
		setCompareB3(PERIOD_SWITCH+0x20);
		// supporess the ZVS pulses
		doStopZVS(true);
		doOutputSetSourceA1None(); // lower switch
		setOutputA1(0); 
		doOutputSetSourceB1None(); //upper switch
		setOutputB1(0); 
	} else {
		if (_rt==0 || _rt==100){ // restore the ZVS pulses (use a boolean bNoZVS)
			doOutputSetSourceA1MasterPer();
			doOutputSetSourceB1MasterCMP1();
			doStopZVS(false);
		}
		doZ1(); // sets A1 and A2
		doZ2(); // sets B1 and B2
		setCompareMaster1(PERIOD_SWITCH - _base);
		setCompareB3(base + baseReduction + _countT2); // (CMP3 is reset source, CMP2 set source initialized to 200)
		setCompareA3(valueA3); //lower switch (CMP3 is reset source, CMP2 set source initialized to 200)
	}
	_rt = valRt;
	//mustUpdateMAB = true;
	doUpdateMAB(); 
}


void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx)
{
	if (mustUpdateMAB) {
		doUpdateMAB(); 
		mustUpdateMAB = false;
	}
}


// command parser
commandAndValue getCommand() {
	commandAndValue result(none, 0);
	char strConsole[SERIAL_BUFFER_SIZE];
	char state = '\0';
	command = none;
	bool hasValue = false;
	int value = 0;
	const char commandList[] = "srazdtvf";
	char *commandListPtr;
	const char numbers12[] = "12";
	const char numbers[] = "0123456789";
	const char compositeCommandList[] = "zdtv";
	bool negativeValue = false;

	if (pSerialInFromConsole->fgetsNonBlocking(strConsole, 48)) {
		int len = strlen(strConsole);
		bool equalFound = false;
		int i;

		for (i=0;i<len;i++){ // skip initial spaces
			char c = strConsole[i];
			if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')){
					break;
				}
		}

		for (;i< len;i++) {
			char c = strConsole[i];
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				break;
			} else 	if (state == '\0') {
				if ((commandListPtr = strchr(commandList, c))) { // look for first char of command
					state = c;
				}
			} else if (equalFound) {
				char * where;
				if ((where = strchr(numbers, c))) {
					value = value * 10 + where - numbers;
				} else if (c == '-') {
					negativeValue = true;
				}
			} else if (command != none) {
				if (c == '=') {
					equalFound = true;
				}
			} else if (strchr(commandList, state)) { //if we have a non 0 state ie. second command char, identify command
				char commandInput[3];
				commandInput[0] = state;
				commandInput[1] = c;
				commandInput[2] = 0;
				for (int j = 0; j < firstCompositeCommand;j++) {
					if (0 == strcmp(commandInput, simpleCommandListStr[j])) {
						command = commandArray[j];
						continue;
					}
				}
				char * where = strchr(numbers12, c);
				int indexIncrement = where - numbers12;
				if ((where = strchr(numbers12, c))) {
					command = commandArray[firstCompositeCommand + (strchr(compositeCommandList, state) - compositeCommandList) * 2 + indexIncrement];
				}
			}
		}
		// end loop, we should have a command
		if (equalFound) {
			result = commandAndValue(command, (negativeValue ? -value : value));
		}
	}
	return result;
}

void processCommand(commandAndValue cv)
{
	switch (cv.command) {
	case st: // display state
		statusDisplay(cv.value==0);
		break;
	case rt:
		setRt(cv.value);
		break;
	case z1:
		setZ1(cv.value);
		break;
	case z2:
		setZ2(cv.value);
		break;
	case d1:
		setD1(cv.value);
		break;
	case d2:
		setD2(cv.value);
		break;
	case v1:
		setV1(cv.value);
		break;
	case v2:
		setV2(cv.value);
		break;
	case t1:
		setT1(cv.value);
		break;
	case t2:
		setT2(cv.value);
		break;
	case tt:
		temperaturesDisplay(cv.value);
		break;
	case ac:
		doAC(cv.value);
		break;
	case ar:
		setBreakerRearm(cv.value);
		break;
	case sm:
		measurementsDisplay(cv.value<10,cv.value<10?cv.value:cv.value-10);
		break;
	case po:
		setMaxPower(cv.value);
		break;
	case fa:
		setFanSpeed(cv.value);
		break;
	case none:
	default:
		;
	}
}

void peekProcessCommand()
{
	commandAndValue cv = getCommand();
	if (cv.command == none) {
		return;
	}
	processCommand(cv);
}


char * my_itoa(int n, int maxVal)
{
	static char message[11];
	int leadingZeros = 0;
	bool negative = false;
	char d;
	int i = 0;
	char nextC = 0;
	message[0] = 0;
	if (n < 0) {
		negative = true;
		n = -n;
		if (n>maxVal/10){
			message[i++] = '-';
			message[i++] = '#';
			message[i] = 0;
			return &message[0];
		}
	}
	if (n > maxVal) {
		message[i++] = '#';
		message[i] = 0;
		return &message[0];
	}
	while (maxVal > 9 && (d = n / maxVal) == 0) {
		leadingZeros++;
		maxVal /= 10;
	}
	while (i < leadingZeros - 1) {
		message[i++] = ' ';
	}
	if (negative != 0) {
		message[i++] = '-';
	} else {
		message[i++] = ' ';
	}
	while (maxVal != 0) {
		d = '0' + n / maxVal;
		message[i++] = d;
		n = n % maxVal;
		maxVal = maxVal / 10;
	}
	message[i] = 0;
	return &message[0];
}
