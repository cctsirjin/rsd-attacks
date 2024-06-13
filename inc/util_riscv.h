#ifndef UTIL_RISCV_H
#define UTIL_RISCV_H

// This one is more universal and may be used to measure time.
#define READ_CSR(reg) ({ \
	unsigned long __tmp; \
	asm volatile ("csrr %0, " #reg : "=r" (__tmp)); \
	__tmp; \
	})


// This one only reads cycles.
#define RDCYCLE ({ \
	int out; \
	asm volatile ("rdcycle %0" : "=r" (out)); \
	out; \
	})

#endif