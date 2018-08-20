#include "Command.h"
#include "Serial.h"
#include <cstring>
#include "Measure.h"
#include "hrtim.h"
#include "stdlib.h"

#define SERIAL_BUFFER_SIZE 50
static char bufferOutConsole[SERIAL_BUFFER_SIZE];
static char bufferInConsole[SERIAL_BUFFER_SIZE];

static SerialOutput SerialOutToConsole(&huart2, bufferOutConsole, SERIAL_BUFFER_SIZE);	 // via USB
static SerialInput SerialInFromConsole(&huart2, bufferInConsole, SERIAL_BUFFER_SIZE);

static SerialOutput* pSerialOutToConsole;
static SerialInput* pSerialInFromConsole;

void initializeCommand()
{
	setRt(10);
	setZ1(150);
	setZ2(150);
	pSerialOutToConsole = &SerialOutToConsole;
	pSerialInFromConsole = &SerialInFromConsole;
	pSerialInFromConsole->initialize(pSerialOutToConsole);
	pSerialOutToConsole->puts("Ready\r\n");
}

typedef enum {
	none,
	st,
	rt,
	z1,
	z2,
	d1, 
	d2, 
	t1,
	t2,
	v1,
	v2
 } command_t;
command_t commandArray[] =  { none,
	st,
	rt,
	z1,
	z2,
	d1, 
	d2, 
	t1,
	t2,
	v1,
	v2
};
command_t command;
/* Commands 2 alphanumerical, = sign, integer 
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
int _countZ1 = 150 * COUNT_PER_NS;
int _countZ2 = 110 * COUNT_PER_NS;
int _countD1 = -20 * COUNT_PER_NS;
int _countD2 = -5 * COUNT_PER_NS;
int _countT1 = 0 * COUNT_PER_NS;
int _countT2 = 0 * COUNT_PER_NS;

#define PERIOD_SWITCH 0x4000
#define PER_CENT 100

char * my_itoa(int n, int maxVal = 1000000)
{
	static char message[11];
	int leadingZeros = 0;
	bool negative = false;
	char d;
	int i = 0;
	char nextC = 0;
	message[0] = 0;
	if (n<0) {
		negative = true;
		n = -n;
	}
	while (maxVal > 9 && (d= n/maxVal) == 0) {
		leadingZeros++;
		maxVal /= 10;
	}
	while (i< leadingZeros - 1){
		message[i++] = ' ';
	}
	if (negative != 0) {
		message[i++] = '-';
	} else {
		message[i++] = ' ';
	}
	while (maxVal != 0){
		d = '0' + n / maxVal;
		message[i++] = d;
		n = n % maxVal;
		maxVal = maxVal / 10;
	}
	message[i + 1] = 0;
	return &message[0];
}
void statusDisplay(void){
	pSerialOutToConsole->puts("    rt\t    z1\t    d1\t    r1\t    t1\t    z2\t    d2\t    r2\t    t2/");
	pSerialOutToConsole->puts(my_itoa(COUNT_PER_NS,10));
	pSerialOutToConsole->puts("->ns\n\r");
	pSerialOutToConsole->puts(my_itoa(_rt));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countZ1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countD1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_base / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countT1 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countZ2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countD2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa((PERIOD_SWITCH - _base) / COUNT_PER_NS));
	pSerialOutToConsole->puts("\t");
	pSerialOutToConsole->puts(my_itoa(_countT2 / COUNT_PER_NS));
	pSerialOutToConsole->puts("\n\r");
}
void setRt(int val){
	_rt = val;
	int base = (PERIOD_SWITCH * val) / PER_CENT;
	_base = base;
	setCompareA3(PERIOD_SWITCH - _base + _countT1);
	setCompareMaster1(PERIOD_SWITCH - _base);
	setCompareB3(base + _countT2);
}
void setZ1(int val){
	int count = val * COUNT_PER_NS;
	_countZ1 = count;
	setCompareA1(_countZ1);
	setCompareA2(_countZ1 + _countD1);
}
void setZ2(int val)
{
	int count = val * COUNT_PER_NS;
	_countZ2 = count;
	setCompareB1(_countZ2);
	setCompareB2(_countZ2 + _countD2);
}
void setD1(int val)
{
	int count = val * COUNT_PER_NS;
	_countD1 = count;
	setCompareA2(_countZ1 + _countD1);
}
void setD2(int val)
{
	int count = val * COUNT_PER_NS;
	_countD2 = count;
	setCompareB2(_countZ2 + _countD2);
}
void setT1(int val)
{
	int count = val * COUNT_PER_NS;
	_countT1 = count;
	setCompareB3(_base + _countT1);
}
void setT2(int val)
{
	int count = val * COUNT_PER_NS;
	_countT2 = count;
	setCompareA3(PERIOD_SWITCH - _base + _countT2);
}
commandAndValue getCommand() {
	commandAndValue result(none, 0);
	char strConsole[SERIAL_BUFFER_SIZE];
	char state = '\0';
	command = none;
	bool hasValue = false;
	int value = 0;
	char commandList[] = "srzdtv";
	char *commandListPtr;
	char simpleCommandList[] = "sr";
	char numbers12[] = "12";
	char numbers[] = "0123456789";
	char compositeCommandList[] = "zdtv";
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
				if ((commandListPtr = strchr(commandList, c))) {
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
			} else if (strchr(commandList, state)) {
				char * where;
				if ((where = strchr(simpleCommandList, state))) {
					command = commandArray[1 + where - simpleCommandList];
				} else if ((where = strchr(numbers12, c))) {
					command = commandArray[z1 + (strchr(compositeCommandList, state) - compositeCommandList) * 2 + where - numbers12];
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
		statusDisplay();
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
