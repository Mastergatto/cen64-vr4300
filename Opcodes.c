/* ============================================================================
 *  Opcodes.c: Opcode types, info, and other data.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Opcodes.h"

/* ============================================================================
 *  Mnemonic and callback tables.
 * ========================================================================= */
const VR4300Function VR4300FunctionTable[NUM_VR4300_OPCODES] = {
#define X(op) VR4300##op,
#include "Opcodes.md"
#undef X
};

#ifndef NDEBUG
const char *VR4300OpcodeMnemonics[NUM_VR4300_OPCODES] = {
#define X(op) #op,
#include "Opcodes.md"
#undef X
};
#endif

