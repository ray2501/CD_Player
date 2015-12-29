#include "misc.h"

int timestring2Int(char *timestring)
{
	int result = 0;
	int minutes = 0, seconds = 0, ticks = 0;
	int stringLength = strlen(timestring);
	char subString[3];

	subString[2] = '\0';

	if(stringLength < 10) {
		subString[0] = timestring[0];
		subString[1] = timestring[1];
	} else {
		subString[0] = timestring[3];
		subString[1] = timestring[4];
	}
	minutes = atoi(subString);

	result = minutes * 60;

	if(stringLength < 10) {
		subString[0] = timestring[3];
		subString[1] = timestring[4];
	} else {
		subString[0] = timestring[6];
		subString[1] = timestring[7];
	}	
	seconds = atoi(subString);

	result+=seconds;

	if(stringLength < 10) {
		subString[0] = timestring[6];
		subString[1] = timestring[7];
	} else {
		subString[0] = timestring[9];
		subString[1] = timestring[10];
	}	
	ticks = atoi(subString);

	if(ticks > 0) {
		++result;
	}

	return(result);
}


void int2TimeString(char *timeString, int value)
{
	int minutes = 0, seconds = 0, ticks = 0;
	minutes = value /60;
	seconds = value%60;
	ticks	 = 0;
	sprintf(timeString, "%.2d:%.2d:%.2d", minutes, seconds, ticks);		
}
