#pragma once

void setRt(int val);
void setZ1(int val);
void setZ2(int val);


#ifdef __cplusplus
extern "C"
{
#endif
	void initializeCommand();
void peekProcessCommand(void);

#ifdef __cplusplus
}
#endif