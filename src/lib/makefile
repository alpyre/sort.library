# Generic Amiga cross-compiler makefile
#  - for m68k OS3 binaries use: make
#  - for ppc OS4 binaries use: make OS=os4
#  - for ppc MorphOS binaries use: make OS=mos

BIN = sort.library
################################################################################
# Target OS
ifndef (OS)
  OS = os3
endif

ifeq ($(OS), os3)
  CPU = -m68020
  CC = m68k-amigaos-gcc
  OPTIONS = -D__USE_INLINE__ -DUSE_SEMAPHORE
  LFLAGS = -s -noixemul -nostdlib -nostartfiles
  OPTIMIZE = -Os
  OBJS = init.o sort.o
else
ifeq ($(OS), os4)
  CPU = -mcpu=powerpc
  CC = ppc-amigaos-gcc
  OPTIONS = -D__USE_INLINE__ -DNO_INLINE_STDARG -DUSE_SEMAPHORE
  LFLAGS = -noixemul -nostdlib -nostartfiles
  OPTIMIZE = -Os
  OBJS = init.o sort.o sort68k.o
else
ifeq ($(OS), mos)
  CPU = -mcpu=powerpc
  CC = ppc-morphos-gcc
  OPTIONS = -D__USE_INLINE__ -DNO_INLINE_STDARG
  LFLAGS = -s -noixemul -nostdlib -nostartfiles -lgcc
  OPTIMIZE = -Os
  OBJS = init.o sort.o
endif
endif
endif
################################################################################
# Common options
WARNINGS = -Wall
DEBUG =
IDIRS = -Iinclude -Iincludes

CFLAGS = $(WARNINGS) $(OPTIMIZE) $(DEBUG) $(CPU) $(OPTIONS) $(IDIRS)
################################################################################

# target 'all' (default target)
all : $(BIN)

# Compile ($@ matches the target; $< matches the first dependent)
%.o : %.c
	$(CC) -c $< $(CFLAGS)

# Link
$(BIN) : $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LFLAGS)

# target 'clean'
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
