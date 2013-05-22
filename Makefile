#  ============================================================================
#   Makefile for *NIX.
#
#   VR4300SIM: NEC VR43xx Processor SIMulator.
#   Copyright (C) 2013, Tyler J. Stachecki.
#   All rights reserved.
#
#   This file is subject to the terms and conditions defined in
#   file 'LICENSE', which is part of this source code package.
#  ============================================================================
TARGET = libvr4300.a

# ============================================================================
#  A list of files to link into the library.
# ============================================================================
SOURCES := $(wildcard *.c)
OBJECTS = $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCES:.c=.o)))

# =============================================================================
#  Build variables and settings.
# =============================================================================
OBJECT_DIR=Objects

BLUE=$(shell tput setaf 4)
PURPLE=$(shell tput setaf 5)
TEXTRESET=$(shell tput sgr0)
YELLOW=$(shell tput setaf 3)

# ============================================================================
#  General build rules.
# ============================================================================
ECHO=/usr/bin/printf "%s\n"
MKDIR = /bin/mkdir -p

AR = ar
DOXYGEN = doxygen

VR4300_FLAGS = -DLITTLE_ENDIAN -DDO_FASTFORWARD -DUSE_X87FPU
WARNINGS = -Wall -Wextra -pedantic -Wunsafe-loop-optimizations

COMMON_CFLAGS = $(WARNINGS) $(VR4300_FLAGS) -std=c99 -march=native -I.
COMMON_CXXFLAGS = $(WARNINGS) $(VR4300_FLAGS) -std=c++0x -march=native -I.
OPTIMIZATION_FLAGS = -flto -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -funsafe-loop-optimizations

ARFLAGS = rcs
RELEASE_CFLAGS = -DNDEBUG -O3 $(OPTIMIZATION_FLAGS)
DEBUG_CFLAGS = -DDEBUG -O0 -ggdb -g3

$(OBJECT_DIR)/%.o: %.c %.h Common.h
	@$(MKDIR) $(OBJECT_DIR)
	@$(ECHO) "$(BLUE)Compiling$(YELLOW): $(PURPLE)$(PREFIXDIR)$<$(TEXTRESET)"
	@$(CC) $(CFLAGS) $< -c -o $@

# ============================================================================
#  Targets.
# ============================================================================
all: CFLAGS = $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
all: $(TARGET)

debug: CFLAGS = $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
debug: $(TARGET)

all-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(RELEASE_CFLAGS)
all-cpp: $(TARGET)
all-cpp: CC = $(CXX)

debug-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(DEBUG_CFLAGS)
debug-cpp: $(TARGET)
debug-cpp: CC = $(CXX)

$(TARGET): $(OBJECTS)
	@$(ECHO) "$(BLUE)Linking$(YELLOW): $(PURPLE)$(PREFIXDIR)$@$(TEXTRESET)"
	@$(AR) $(ARFLAGS) $@ $^

.PHONY: clean documentation inspect inspect-cpp

clean:
	@$(ECHO) "$(BLUE)Cleaning libvr4300...$(TEXTRESET)"
	@$(RM) $(OBJECTS) $(TARGET)

inspect: $(TARGET)
	objdump -d $< | less

inspect-cpp: $(TARGET)
	objdump -d $< | c++filt | less

