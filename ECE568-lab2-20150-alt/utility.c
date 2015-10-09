#include "utility.h"

/*
 * Return a pointer to the memory address containing 
 * a copy of the string from source
 *
 * source : const char* - the source string
 * return : char* - the memory address of the new string
 */
char* strCopy(const char* source){
	long len = strlen(source) + 1;
	char* destination = (char*)myMalloc(sizeof(char)*len, NULL);
	if (destination == NULL) {
		perror("strCopy error");
		exit(1);
	}
	strcpy(destination, source);
	return destination;
}

/*
 * Return a pointer pointing to a new matrix (using malloc)
 *
 * rowCount : int - the total number of rows in the new matrix
 * columnCount : int - the total number of columns in the new matrix
 * return : int** - the pointer pointing to the new matrix
 */
int** createIntMatrix(int rowCount, int columnCount){
	int i;
	int** result = myMalloc(sizeof(int*) * rowCount, NULL);
 	for (i = 0 ; i < rowCount ; i++){
		result[i] = myMalloc(sizeof(int) * columnCount, NULL);
	}
	return result;
}

/* Return a pointer to the new string following the format and arguments
 * 
 * format : const char* - the format
 * ... : va_list - the list of arguments
 * return : char* - the pointer to the new string
 */
char* buildString( const char * format, ... ){
	int length;
	char* outputString;
	va_list args;
	va_start (args, format);
	length = vprintf(format, args);
	va_end (args);
	outputString = myMalloc(sizeof(char) * (length + 1), NULL);
	va_start (args, format);
	vsprintf(outputString, format, args);
	va_end (args);
	return outputString;
}

