XCFLAGS = -g
#XCFLAGS = -g -mcmodel=medany -l -std=gnu99 -O0 -g -fno-common -fno-builtin-printf -Wall -I$(INC) -Wno-unused-function -Wno-unused-variable
SRCS = code.c
include ../BuildC.inc.mk

# Folders
#SRC:=src
INC:=inc
LNK:=link
OBJ:=obj
BIN:=bin
DMP:=dump
DEP:=dep

# Commands and flags
#CC:=riscv32-unknown-elf-gcc
#OBJDUMP:=riscv32-unknown-elf-objdump -S
#LDFLAGS:=-static -lgcc

# -lgcc: linker parameter that tells the linker to link against libgcc
# https://stackoverflow.com/questions/28455999/do-i-have-to-link-the-files-with-lgcc

#CFLAGS=-mcmodel=medany -l -std=gnu99 -O0 -g -fno-common -fno-builtin-printf -Wall -I$(INC) -Wno-unused-function -Wno-unused-variable

CFLAGS = \
        -l \
		-std=gnu99 \
		-O0
		-fno-common \
        -g \
		-I$(INC) \
        -fno-zero-initialized-in-bss \
        -ffreestanding \
		-fno-builtin-printf \
		-Wall \
		-Wno-unused-function \
		-Wno-unused-variable \
        -mstrict-align \
        -march=rv32imf \
        -mabi=ilp32f
 
LDFLAGS= \
        -static \
		-lgcc
 
LIBC =    
LIBGCC = \
        -L$(RSD_GCC_NATIVE)/../lib/gcc/${RSD_GCC_NAME}/$(RSD_GCC_VERSION) \
        -lgcc \
        -lgcov \
        -L$(RSD_GCC_NATIVE)/../${RSD_GCC_NAME}/lib \
        -lm


# Universal GCC options: -g debugging. -l library
# https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html

# -O0: Options That Control Optimization
# >Reduce compilation time and make debugging produce the expected results. This is the default. 
# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html

# -mcmodel=medany: RISC-V Options of GCC.
# https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html

# -std=gnu99 (for C99 with GNU extensions) :
# https://gcc.gnu.org/onlinedocs/gcc/Standards.html

# -fno-common: specifies that the compiler places uninitialized global variables in the BSS section of the object file.
# https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html

# -fno-builtin-function
# https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html

# -W: warning options.
# https://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Warning-Options.html

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEP)/$*.d

# DEPFLAGS: Auto-Dependency Generation:
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# Options Controlling the Preprocessor:
# https://gcc.gnu.org/onlinedocs/gcc-5.2.0/gcc/Preprocessor-Options.html