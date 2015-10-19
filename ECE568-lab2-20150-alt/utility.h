#ifndef __UTILITY_H__
#define __UTILITY_H__
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	#include "myLib.h"	

	char* strCopy(const char* source);
	int** createIntMatrix(int rowCount, int columnCount);
	char* buildString( const char * format, ... );
	char* addString(char* stringA, char* stringB, int freeSignal);
	void printErrorThenQuit(const char* errorMessage);
#endif 
