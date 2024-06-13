#ifndef CACHE_H
#define CACHE_H

// L1 data cache mapping for T-Head TH1520 SoC (quad XuanTie C910 cores)
// L1 data Cache Capacity(C): "Each core contains 64KB I cache amd 64KB D Cache." Refer to https://github.com/sipeed/sipeed_wiki/blob/main/docs/hardware/en/lichee/th1520/lpi4a/1_intro.md
#define L1_DCACHE_WAYS 2 // Degree of associativity N = 2. i.e. 2-way set associative. Refer to https://www.riscfive.com/2023/03/09/t-head-xuantie-c910-risc-v/
#define L1_DCACHE_BLOCK_BYTES 64 // b = 64Byte. "Fixed cache line length of 64 bytes." Refer to https://www.riscfive.com/2023/03/09/t-head-xuantie-c910-risc-v/
#define L1_DCACHE_BLOCK_BITS 6 // = log2(b).
#define L1_DCACHE_SETS 512 // S = 512. S=B/N=C/Nb. Here C=64KiB, L1_DCACHE_WAYS=N=2, b=64B, so we have S=64KiB/(2x64B)=(1/2)K=512.
#define L1_DCACHE_SETS_BITS 9 // = log2(S)
// Also search "i-cache-sets" in https://lore.kernel.org/linux-riscv/20230617161529.2092-6-jszhang@kernel.org/
#define L1_DCACHE_CAPACITY_BYTES (L1_DCACHE_SETS*L1_DCACHE_WAYS*L1_DCACHE_BLOCK_BYTES) // Cache Capacity C=Bb=SNb=1K*64Byte=64KiB
#define FULL_MASK 0xFFFFFFFFFFFFFFFF // The address size is 64 bits.
/**
 * Sv39 virtual memory translation:
 * Instruction fetch addresses and load and store effective addresses, which are 64 bits,
 * must have bits 63–39 all equal to bit 38, or else a page-fault exception will occur.
 */
#define OFFSET_MASK (~(FULL_MASK << L1_DCACHE_BLOCK_BITS)) // Offset bits are 0 to (L1_DCACHE_BLOCK_BITS - 1).
// After this definition, only offset bits remain 1, and they are used to discover existence of any cache set in use.
#define TAG_MASK (FULL_MASK << (L1_DCACHE_SETS_BITS + L1_DCACHE_BLOCK_BITS))
// After this definition, only tag bits remain 1, and they are used to align memory.
#define SET_MASK (~(TAG_MASK | OFFSET_MASK))
// After this definition, only set bits remain 1, and they are used to clear tag and offset field of input addr.

/* ----------------------------------
 * | Cache fields for a mapped memory address |
 * The LSBs (Least Significant Bits) of the address specify which set holds the data.
 * - Block offset. 64-bit RISC-V processors do not need "byte offset" for 64-bit addresses.
 * - The next several bits are called the "set bits" because they indicate the set to which the address maps.
 * The remaining MSBs (Most Significant Bits) are the "tag" and indicate which of the many possible addresses is held in that set.
 * ----------------------------------
 * | Tag (within a set) |  Set bits (index of set)  |       Block offset     |
 * ----------------------------------
 * |   <--Remaining-->  |       <--log2(S) -->      |     <--log2(b) -->    |
 * ----------------------------------
 */

// Set up an empty array to put into the cache during the following cache flush.
// This aims to guarantee a contiguous set of addresses which is at least the size of cache.
// TBD: Investigate how to determine this MULTIPLIER.
#define MULTIPLIER 10
// If your source codes are unable to flush the cache thoroughly, you may try increasing this MULTIPLIER.
// But, of course, that will cause longer execution time of the final binary programs.
uint8_t dummyMem[MULTIPLIER * L1_DCACHE_CAPACITY_BYTES];
// Temporary variable.
uint8_t flush_junk = 0;
/**
 * Flush the cache of the address given since RV64 does not have an x86 clflush type instruction.
 * Clears any set that has the same set bits as the input address range.
 * Note: This does not work if you are trying to flush dummyMem out of the cache.
 * @param memAddr starting address to clear the cache
 * @param memSize size of the data to remove in bytes
 */
void flushCache(uint64_t memAddr, uint64_t memSize){
    //printf("Flushed memAddr(0x%x) tag(0x%x) set(0x%x) off(0x%x) memSize(%d)\n", memAddr, (memAddr & TAG_MASK) >> (L1_DCACHE_SETS_BITS + L1_DCACHE_BLOCK_BITS), (memAddr & SET_MASK) >> L1_DCACHE_BLOCK_BITS, memAddr & OFFSET_MASK, memSize);

    // Find out the quantity of blocks that need to be cleared.
    uint64_t numSetsClear = memSize >> L1_DCACHE_BLOCK_BITS; // This operation removes block bits and shifts set bits to the right.
	//  Offset bits other than all 0s indicate that it is not the beginning block of a set, so additional clearing is required.
    if ((memSize & OFFSET_MASK) != 0){
        numSetsClear += 1;
    }
	// Flush the entire cache with no rollover, which makes this flushCache function finish faster.
    if (numSetsClear > L1_DCACHE_SETS){
        numSetsClear = L1_DCACHE_SETS;
    }

    //printf("numSetsClear(%d)\n", numSetsClear);

    // This memory address alignedMem is the start of a contiguous set of memory that will fit inside the cache.
    // Thus it has the following properties:
    // 1. dummyMem <= alignedMem < dummyMem + sizeof(dummyMem) (since set bits and offset are cleared)
    // 2. alignedMem has set bits = 0 and offset = 0, only tag bits remain.
    uint64_t alignedMem = (((uint64_t)&dummyMem) + L1_DCACHE_CAPACITY_BYTES) & TAG_MASK;
	// "&" is the address-of operator. The type of the result is "pointer to the operand."
    //printf("alignedMem(0x%x)\n", alignedMem);

    for (uint64_t i = 0; i < numSetsClear; ++i){
        // Combined with the for loop to move across the sets that you want to flush.
        uint64_t setOffset = (((memAddr & SET_MASK) >> L1_DCACHE_BLOCK_BITS) + i) << L1_DCACHE_BLOCK_BITS;
        //printf("setOffset(0x%x)\n", setOffset);

        // There are N=L1_DCACHE_WAYS in a set to flush. And it needs to be repeated for 5 times since we have an extended dummyMem.
        for(uint64_t j = 0; j < ((MULTIPLIER-1)*L1_DCACHE_WAYS); ++j){
            // offset to reaccess the set
            uint64_t wayOffset = j << (L1_DCACHE_BLOCK_BITS + L1_DCACHE_SETS_BITS);
            //printf("wayOffset(0x%x)\n", wayOffset);

            // The processor will fetch needed (but empty, this property is important) data into the cache
			// as the following expression shows, which, due to same set bits and tags, evicts previous data.
            flush_junk = *((uint8_t*)(alignedMem + setOffset + wayOffset));
			// * indirection or dereferencing operator accesses the object the pointer points to.
            //printf("evict read(0x%x)\n", alignedMem + setOffset + wayOffset);
        }
    }
}

#endif
