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

ifeq ($(OS),windows)
OBJECTS = $(addprefix $(OBJECT_DIR)\, $(notdir $(SOURCES:.c=.o)))
else
OBJECTS = $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCES:.c=.o)))
endif

# =============================================================================
#  Build variables and settings.
# =============================================================================
OBJECT_DIR=Objects

# ============================================================================
#  Build rules and flags.
# ============================================================================
BLUE=$(shell tput setaf 4)
PURPLE=$(shell tput setaf 5)
TEXTRESET=$(shell tput sgr0)
YELLOW=$(shell tput setaf 3)

ECHO=/usr/bin/printf "%s\n"
MKDIR = /bin/mkdir -p

ifeq ($(OS),windows)
CC=gcc.exe
CXX=g++.exe

BLUE=
PURPLE=
TEXTRESET=
YELLOW=

ECHO=echo
MAYBE=if not exist
MKDIR=mkdir
RM=del /q /s 1>NUL 2>NUL
endif

AR = ar
DOXYGEN = doxygen

VR4300_FLAGS = -DLITTLE_ENDIAN -DDO_FASTFORWARD -DUSE_X87FPU
WARNINGS = -Wall -Wextra -pedantic

COMMON_CFLAGS = $(WARNINGS) $(VR4300_FLAGS) -std=c99 -march=native -I.
COMMON_CXXFLAGS = $(WARNINGS) $(VR4300_FLAGS) -std=c++0x -march=native -I.
OPTIMIZATION_FLAGS = -flto -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -funsafe-loop-optimizations

ARFLAGS = rcs
RELEASE_CFLAGS = -DNDEBUG -O3 $(OPTIMIZATION_FLAGS)
DEBUG_CFLAGS = -DDEBUG -O0 -ggdb -g3

# ============================================================================
#  Build targets.
# ============================================================================
.PHONY: all all-cpp clean debug debug-cpp

all: CFLAGS = $(COMMON_CFLAGS) $(RELEASE_CFLAGS) $(VR4300_FLAGS)
all: $(TARGET)

debug: CFLAGS = $(COMMON_CFLAGS) $(DEBUG_CFLAGS) $(VR4300_FLAGS)
debug: $(TARGET)

all-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(RELEASE_CFLAGS) $(VR4300FLAGS)
all-cpp: $(TARGET)
all-cpp: CC = $(CXX)

debug-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(DEBUG_CFLAGS) $(VR4300FLAGS)
debug-cpp: $(TARGET)
debug-cpp: CC = $(CXX)

clean:
ifeq ($(OS),windows)
	@$(ECHO) $(BLUE)Cleaning libvr4300...$(TEXTRESET)
else
	@$(ECHO) "$(BLUE)Cleaning libvr4300...$(TEXTRESET)"
endif
	@$(RM) $(OBJECTS) $(TARGET)

# ============================================================================
#  Build rules.
# ============================================================================
ifeq ($(OS),windows)
$(TARGET): $(OBJECTS)
	@$(ECHO) $(BLUE)Linking$(YELLOW): $(PURPLE)$(PREFIXDIR)$@$(TEXTRESET)
	@$(AR) $(ARFLAGS) $@ $^

$(OBJECT_DIR)\\%.o: %.c %.h Common.h
	@$(MAYBE) $(OBJECT_DIR) $(MKDIR) $(OBJECT_DIR)
	@$(ECHO) $(BLUE)Compiling$(YELLOW): $(PURPLE)$(PREFIXDIR)$<$(TEXTRESET)
	@$(CC) $(CFLAGS) $< -c -o $@
else
$(TARGET): $(OBJECTS)
	@$(ECHO) "$(BLUE)Linking$(YELLOW): $(PURPLE)$(PREFIXDIR)$@$(TEXTRESET)"
	@$(AR) $(ARFLAGS) $@ $^

$(OBJECT_DIR)/%.o: %.c %.h Common.h
	@$(MKDIR) $(OBJECT_DIR)
	@$(ECHO) "$(BLUE)Compiling$(YELLOW): $(PURPLE)$(PREFIXDIR)$<$(TEXTRESET)"
	@$(CC) $(CFLAGS) $< -c -o $@
endif

