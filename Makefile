###############################################################################
# Makefile for riscv-spectre
###########################
# Makes baremetal executables to run on RISC-V implementations.
###############################################################################

# Folders
SRC:=src
INC:=inc
LNK:=link
OBJ:=obj
BIN:=bin
DMP:=dump
DEP:=dep
## For Verilator
SRCRV:=src/verilator
OBJRV:=objrv
BINRV:=binrv
DMPRV:=dumprv

# Commands and flags
CC:=riscv64-unknown-elf-gcc
OBJDUMP:=riscv64-unknown-elf-objdump -S
LDFLAGS:=-static -lgcc
## For Verilator
CCRV:=riscv64-unknown-elf-gcc
OBJDUMPRV:=riscv64-unknown-elf-objdump -S
LDFLAGSRV:=-static -nostdlib -nostartfiles -lgcc

# -lgcc: linker parameter that tells the linker to link against libgcc
# https://stackoverflow.com/questions/28455999/do-i-have-to-link-the-files-with-lgcc

CFLAGS=-mcmodel=medany -l -std=gnu99 -O0 -g -fno-common -fno-builtin-printf -Wall -I$(INC) -Wno-unused-function -Wno-unused-variable

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

RM=rm -rf

# Programs to compile
# Produces results like bin/foo.riscv bin/bar.riscv
PROGRAMS=Spectre-BCB Spectre-BTI Spectre-RSB Spectre-SSB
BINS=$(addprefix $(BIN)/,$(addsuffix .riscv,$(PROGRAMS)))
DUMPS=$(addprefix $(DMP)/,$(addsuffix .dump,$(PROGRAMS)))

## For Verilator
BINSRV=$(addprefix $(BINRV)/,$(addsuffix .riscvrv,$(PROGRAMS)))
DUMPSRV=$(addprefix $(DMPRV)/,$(addsuffix .dumprv,$(PROGRAMS)))


.PHONY: all allrv
all: $(BINS) $(DUMPS)
bin: $(BINS)
dumps: $(DUMPS)

## For Verilator
allrv: $(BINSRV) $(DUMPSRV)
binrv: $(BINSRV)
dumpsrv: $(DUMPSRV)

# Option -c: will compile or assemble the source files, but do not link.
# Option -o file: places the primary output in file file.
# https://gcc.gnu.org/onlinedocs/gcc/Overall-Options.html

# Automatic variables $< and $@
# $@ is the the file name of the target of the rule. Here for the object file name or the executable.
# $< is the name of the first prerequisite. Here for the source file or the object file name.

# Build object files
$(OBJ)/%.o: $(SRC)/%.S
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -D__ASSEMBLY__=1 -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	@mkdir -p $(DEP)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

## For Verilator
$(OBJRV)/%.o: $(SRCRV)/%.S
	@mkdir -p $(OBJRV)
	$(CCRV) $(CFLAGS) -D__ASSEMBLY__=1 -c $< -o $@

$(OBJRV)/%.o: $(SRCRV)/%.c
	@mkdir -p $(OBJRV)
	@mkdir -p $(DEP)
	$(CCRV) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Build executable
$(BIN)/%.riscv: $(OBJ)/%.o $(OBJ)/swStackGadget.o
#$(BIN)/%.riscv: $(OBJ)/%.o
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) $< $(OBJ)/swStackGadget.o -o $@
#	$(CC) $(LDFLAGS) $< -o $@

## For Verilator
$(BINRV)/%.riscvrv: $(OBJRV)/%.o $(OBJRV)/crt.o $(OBJRV)/syscalls.o $(OBJRV)/swStackGadget.o $(LNK)/link.ld
	@mkdir -p $(BINRV)
	$(CCRV) -T $(LNK)/link.ld $(LDFLAGSRV) $< $(OBJRV)/crt.o $(OBJRV)/swStackGadget.o $(OBJRV)/syscalls.o -o $@

# Build dump
$(DMP)/%.dump: $(BIN)/%.riscv
	@mkdir -p $(DMP)
	$(OBJDUMP) -D $< > $@

## For Verilator
$(DMPRV)/%.dumprv: $(BINRV)/%.riscvrv
	@mkdir -p $(DMPRV)
	$(OBJDUMPRV) -D $< > $@
	
# Keep the temporary .o files
.PRECIOUS: $(OBJ)/%.o
.PRECIOUS: $(OBJRV)/%.o

# Remove all generated files
clean:
	rm -rf $(BIN) $(OBJ) $(DMP) $(DEP) $(BINRV) $(OBJRV) $(DMPRV)

# Include dependencies
# include tells make to suspend reading the current makefile and read one or more other makefiles before continuing.
# -include acts like include in every way except that there is no error (not even a warning) if any of the filenames (or any prerequisites of any of the filenames) do not exist or cannot be remade.
-include $(addprefix $(DEP)/,$(addsuffix .d,$(PROGRAMS)))
