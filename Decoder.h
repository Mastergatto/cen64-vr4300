/* ============================================================================
 *  Decoder.h: Instruction decoder definitions.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__DECODER_H__
#define __VR4300__DECODER_H__
#include "Common.h"
#include "Opcodes.h"

#define GET_RS(opcode) ((opcode) >> 21 & 0x1F)
#define GET_RT(opcode) ((opcode) >> 16 & 0x1F)
#define GET_RD(opcode) ((opcode) >> 11 & 0x1F)

#define GET_FS(opcode) ((opcode) >> 11 & 0x1F)
#define GET_FT(opcode) ((opcode) >> 16 & 0x1F)
#define GET_FD(opcode) ((opcode) >>  6 & 0x1F)

/* opcode_t->infoFlags */
#define OPCODE_INFO_NONE (0)
#define OPCODE_INFO_BRANCH (1 << 1)         /* Branch insns.          */
#define OPCODE_INFO_BREAK (1 << 2)          /* Break instruction.     */
#define OPCODE_INFO_CP0 (1 << 3)            /* CP0 insns (CT,MF,MT).  */
#define OPCODE_INFO_CP2 (1 << 4)            /* CP2 insns (CT,MF,MT).  */
#define OPCODE_INFO_LOAD (1 << 5)           /* Scalar load insns.     */
#define OPCODE_INFO_NEED_RS (1 << 6)        /* Requires rs/vs.        */
#define OPCODE_INFO_NEED_RT (1 << 7)        /* Requires rt/vt.        */
#define OPCODE_INFO_STORE (1 << 8)          /* Scalar store insns.    */
#define OPCODE_INFO_WRITE_LR (1 << 9)       /* Can write to link reg. */
#define OPCODE_INFO_WRITE_RD (1 << 10)      /* Writes to rd/vd.       */
#define OPCODE_INFO_WRITE_RT (1 << 11)      /* Writes to rt/vt.       */

/* Decoder results. */
struct VR4300Opcode {
  enum VR4300OpcodeID id;
  uint32_t flags;
};

/* Escape table data. */
struct VR4300OpcodeEscape {
  const struct VR4300Opcode *table;
  uint32_t shift, mask;
};

const struct VR4300Opcode* VR4300DecodeInstruction(uint32_t);
void VR4300InvalidateOpcode(struct VR4300Opcode *);

#endif

