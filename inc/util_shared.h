#ifndef UTIL_SHARED_H
#define UTIL_SHARED_H

/* ANSI escape color codes */
#define ANSI_CODE_RESET      "\033[00m"
#define ANSI_CODE_BOLD       "\033[1m"
#define ANSI_CODE_DARK       "\033[2m"
#define ANSI_CODE_UNDERLINE  "\033[4m"
#define ANSI_CODE_BLINK      "\033[5m"
#define ANSI_CODE_REVERSE    "\033[7m"
#define ANSI_CODE_CONCEALED  "\033[8m"
#define ANSI_CODE_GRAY       "\033[30m"
#define ANSI_CODE_GREY       "\033[30m"
#define ANSI_CODE_RED        "\033[31m"
#define ANSI_CODE_GREEN      "\033[32m"
#define ANSI_CODE_YELLOW     "\033[33m"
#define ANSI_CODE_BLUE       "\033[34m"
#define ANSI_CODE_MAGENTA    "\033[35m"
#define ANSI_CODE_CYAN       "\033[36m"
#define ANSI_CODE_WHITE      "\033[37m"
#define ANSI_CODE_BG_GRAY    "\033[40m"
#define ANSI_CODE_BG_GREY    "\033[40m"
#define ANSI_CODE_BG_RED     "\033[41m"
#define ANSI_CODE_BG_GREEN   "\033[42m"
#define ANSI_CODE_BG_YELLOW  "\033[43m"
#define ANSI_CODE_BG_BLUE    "\033[44m"
#define ANSI_CODE_BG_MAGENTA "\033[45m"
#define ANSI_CODE_BG_CYAN    "\033[46m"
#define ANSI_CODE_BG_WHITE   "\033[47m"

/**
 * Reads in inArray and corresponding size, as well as outIdxArrays top two idx's and their
 * corresponding values in the inArray, which has the highest values.
 *
 * @input inArray array of values to find the top two maxs
 * @input inArraySize size of the inArray array in entries
 * @inout outIdxArray array holding the idxs of the top two values ([0] idx has the larger value in inArray array)
 * @inout outValArray array holding the top two values ([0] has the larger value)
 */
void topTwoIdx(uint64_t* inArray, uint64_t inArraySize, uint8_t* outIdxArray, uint64_t* outValArray){
	
	outValArray[0] = 0;
	outValArray[1] = 0;

	for (uint64_t i = 0; i < inArraySize; i++){
		if (inArray[i] > outValArray[0]){
			outValArray[1] = outValArray[0];
			outValArray[0] = inArray[i];
			outIdxArray[1] = outIdxArray[0];
			outIdxArray[0] = i;
		}
		else if (inArray[i] > outValArray[1]){
			outValArray[1] = inArray[i];
			outIdxArray[1] = i;
		}
	}
}

void dynamicInputString(char* defaultString, int maxStringLength, char* outputString){
	
	int userStringLength;
	
	printf("------ Customized Settings of Target String ------\n");
	printf("Please enter a string (in extended ASCII with "ANSI_CODE_RED"a maximum of %d characters"ANSI_CODE_RESET"), \nor enter nothing to apply default settings. (default: %s)\n", maxStringLength, defaultString);
	printf("Part of input string that exceeds the maximum length will get chopped.\n(You can manually adjust #define MAX_STRING_LENGTH_FACTOR in the source code file.)\n");
	printf("Now provide your string: ");
	
	// Read the string using fgets and remove potential trailing newline.
	fgets(outputString, maxStringLength, stdin);
	outputString[strcspn(outputString, "\n")] = '\0';	
	userStringLength = strlen(outputString);
	
	if (userStringLength == 0) {
		printf("Using default string: "ANSI_CODE_YELLOW"%s"ANSI_CODE_RESET"\n", defaultString);
		strcpy(outputString, defaultString);
	} else {
		printf("Using customized string: "ANSI_CODE_YELLOW"%s"ANSI_CODE_RESET"\n", outputString);
	
	}

}
#endif
