/* ============================================================================
 *  EXStage.c: Execute stage.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CPU.h"
#include "Decoder.h"
#include "DCStage.h"
#include "EXStage.h"
#include "Opcodes.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cassert>
#include <cstddef>
#include <cstring>
#else
#include <assert.h>
#include <stddef.h>
#include <string.h>
#endif

/* ============================================================================
 *  Instruction: ADD (Add)
 *  TODO: Integer overflow exception.
 * ========================================================================= */
void
VR4300ADD(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rs + rt);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: ADDI (Add Immediate)
 *  TODO: Integer overflow exception.
 * ========================================================================= */
void
VR4300ADDI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  int64_t result = (int32_t) (rs + imm);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: ADDIU (Add Immediate Unsigned)
 * ========================================================================= */
void
VR4300ADDIU(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  int64_t result = (int32_t) (rs + imm);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: ADDU (Add Unsigned)
 * ========================================================================= */
void
VR4300ADDU(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rs + rt);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: AND (And)
 * ========================================================================= */
void
VR4300AND(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = rs & rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: ANDI (And Immediate)
 * ========================================================================= */
void
VR4300ANDI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  uint16_t imm = rfexLatch->iw;

  exdcLatch->result.data = rs & imm;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: BC2 (Branch On Coprocessor 2 Instructions)
 * ========================================================================= */
void
VR4300BC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BC2.");
}

/* ============================================================================
 *  Instruction: BEQ (Branch On Equal)
 * ========================================================================= */
void
VR4300BEQ(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if (rs != rt)
    return;

#ifdef DO_FASTFORWARD
  if (address == 0xFFFFFFFFFFFFFFF8 && rs == 0 && rt == 0)
    if (vr4300->pipeline.faultManager.killStage == -1)
    vr4300->pipeline.faultManager.killStage = VR4300_PIPELINE_STAGE_WB;
  //assert(0);
#endif

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BEQL (Branch On Equal Likely)
 * ========================================================================= */
void
VR4300BEQL(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if (rs != rt) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BGEZ (Branch On Greater Than Or Equal To Zero)
 * ========================================================================= */
void
VR4300BGEZ(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs < 0)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BGEZAL (Branch On Greater Than Or Equal To Zero And Link)
 * ========================================================================= */
void
VR4300BGEZAL(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  exdcLatch->result.data = icrfLatch->pc + 4;
  exdcLatch->result.dest = VR4300_REGISTER_RA;

  if ((int64_t) rs < 0)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BGEZALL (Branch On Greater Than Or Equal TO Zero And Link L.)
 * ========================================================================= */
void
VR4300BGEZALL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BGEZALL.");
}

/* ============================================================================
 *  Instruction: BGEZL (Branch On Greater Than Or Equal To Zero Likely)
 * ========================================================================= */
void
VR4300BGEZL(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs < 0) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BGTZ (Branch On Greater Than Zero)
 * ========================================================================= */
void
VR4300BGTZ(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs <= 0)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BGTZL (Branch On Greater Than Zero Likely)
 * ========================================================================= */
void
VR4300BGTZL(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs <= 0) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BLEZ (Branch On Less Than Or Equal To Zero)
 * ========================================================================= */
void
VR4300BLEZ(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs > 0)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BLEZL (Branch On Less Than Or Equal To Zero Likely)
 * ========================================================================= */
void
VR4300BLEZL(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs > 0) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BLTZ (Branch On Less Than Zero)
 * ========================================================================= */
void
VR4300BLTZ(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs >= 0)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BLTZAL (Branch On Less Than Zero And Link)
 * ========================================================================= */
void
VR4300BLTZAL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BLTZAL.");
}

/* ============================================================================
 *  Instruction: BLTZALL (Branch On Less Than Zero And Link Likely)
 * ========================================================================= */
void
VR4300BLTZALL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BLTZALL.");
}

/* ============================================================================
 *  Instruction: BLTZL (Branch On Less Than Zero Likely)
 * ========================================================================= */
void
VR4300BLTZL(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if ((int64_t) rs >= 0) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BNE (Branch On Not Equal)
 * ========================================================================= */
void
VR4300BNE(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if (rs == rt)
    return;

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BNEL (Branch On Not Equal Likely)
 * ========================================================================= */
void
VR4300BNEL(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if (rs == rt) {
    icrfLatch->iwMask = 0;
    return;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: BREAK (Breakpoint)
 * ========================================================================= */
void
VR4300BREAK(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BREAK.");
}

/* ============================================================================
 *  Instruction: CACHE (Cache Operation)
 * ========================================================================= */
void
VR4300CACHE(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  /*debug("Unimplemented function: CACHE.");*/
}

/* ============================================================================
 *  Instruction: CFC2 (Move Control From Coprocessor 2)
 * ========================================================================= */
void
VR4300CFC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: CFC2.");
}

/* ============================================================================
 *  Instruction: COP1 (Coprocessor 1 Instructions) 
 * ========================================================================= */
void
VR4300COP1(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: COP1.");
}

/* ============================================================================
 *  Instruction: COP2 (Coprocessor 2 Instructions)
 * ========================================================================= */
void
VR4300COP2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: COP2.");
}

/* ============================================================================
 *  Instruction: CTC2 (Move Control to Coprocessor 2)
 * ========================================================================= */
void
VR4300CTC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: CTC2.");
}

/* ============================================================================
 *  Instruction: DADD (Doubleword Add)
 * ========================================================================= */
void
VR4300DADD(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DADD.");
}

/* ============================================================================
 *  Instruction: DADDI (Doubleword Add Immediate)
 * ========================================================================= */
void
VR4300DADDI(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DADDI.");
}

/* ============================================================================
 *  Instruction: DADDIU (Doubleword Add Immediate Unsigned)
 * ========================================================================= */
void
VR4300DADDIU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DADDIU.");
}

/* ============================================================================
 *  Instruction: DADDU (Doubleword Add Unsigned)
 * ========================================================================= */
void
VR4300DADDU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DADDU.");
}

/* ============================================================================
 *  Instruction: DDIV (Doubleword Divide)
 *  TODO: Do we write LO/HI here, or wait until WB?
 * ========================================================================= */
void
VR4300DDIV(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DDIV.");
}

/* ============================================================================
 *  Instruction: DDIVU (Doubleword Divide Unsigned)
 *  TODO: Do we write LO/HI here, or wait until WB?
 * ========================================================================= */
void
VR4300DDIVU(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  assert(rt != 0 && "VR4300DDIVU: Quotient is zero?");

  if (likely(rt != 0)) {
    uint64_t div = rs / rt;
    uint64_t mod = rs % rt;

    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }
}

/* ============================================================================
 *  Instruction: DIV (Divide)
 * ========================================================================= */
void
VR4300DIV(struct VR4300 *vr4300, uint64_t _rs, uint64_t _rt) {
  assert(_rt != 0 && "VR4300DIV: Quotient is zero?");

  if (likely(_rt != 0)) {
    int64_t rs = (int32_t) _rs;
    int64_t rt = (int32_t) _rt;

    int64_t div = (int32_t) (rs / rt);
    int64_t mod = (int32_t) (rs % rt);

    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }
}

/* ============================================================================
 *  Instruction: DIVU (Divide Unsigned)
 * ========================================================================= */
void
VR4300DIVU(struct VR4300 *vr4300, uint64_t _rs, uint64_t _rt) {
  assert(_rt != 0 && "VR4300DIV: Quotient is zero?");

  if (likely(_rt != 0)) {
    uint64_t rs = (uint32_t) _rs;
    uint64_t rt = (uint32_t) _rt;

    uint64_t div = (uint32_t) (rs / rt);
    uint64_t mod = (uint32_t) (rs % rt);

    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }
}

/* ============================================================================
 *  Instruction: DMFC1 (Doubleword Move From Coprocessor 1)
 * ========================================================================= */
void
VR4300DMFC1(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMFC1.");
}

/* ============================================================================
 *  Instruction: DMFC2 (Doubleword Move From Coprocessor 1)
 * ========================================================================= */
void
VR4300DMFC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMFC2.");
}

/* ============================================================================
 *  Instruction: DMTC1 (Doubleword Move To Coprocessor 1)
 * ========================================================================= */
void
VR4300DMTC1(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMTC1.");
}

/* ============================================================================
 *  Instruction: DMTC2 (Doubleword Move To Coprocessor 2)
 * ========================================================================= */
void
VR4300DMTC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMTC2.");
}

/* ============================================================================
 *  Instruction: DMULT (Doubleword Multiply)
 * ========================================================================= */
void
VR4300DMULT(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMULT.");
}

/* ============================================================================
 *  Instruction: DMULTU (Doubleword Multiply Unsigned)
 *  TODO: Do we write LO/HI here, or wait until WB?
 * ========================================================================= */
void
VR4300DMULTU(struct VR4300 *vr4300, uint64_t _rs, uint64_t _rt) {
  uint64_t lo, hi;

#if(defined(__GNUC__) && (defined(__x86__) || defined(__x86_64__)))
  __uint128_t rs = _rs;
  __uint128_t rt = _rt;
  __uint128_t result;

  result = rs * rt;

  lo = result;
  hi = result >> 64;

#else
#warning "Unimplemented function: VR4300DMULTU."
#endif

  vr4300->regs[VR4300_REGISTER_LO] = lo;
  vr4300->regs[VR4300_REGISTER_HI] = hi;
}

/* ============================================================================
 *  Instruction: DSLL (Doubleword Shift Left Logical)
 * ========================================================================= */
void
VR4300DSLL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rfexLatch->iw >> 6 & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  uint64_t result = rt << sa;

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: DSLLV (Doubleword Shift Left Logical Variable)
 * ========================================================================= */
void
VR4300DSLLV(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSLLV.");
}

/* ============================================================================
 *  Instruction: DSLL32 (Doubleword Shift Left Logical + 32)
 * ========================================================================= */
void
VR4300DSLL32(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = (rfexLatch->iw >> 6 & 0x1F) + 32;
  unsigned dest = GET_RD(rfexLatch->iw);
  uint64_t result = rt << sa;

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: DSRA (Doubleword Shift Right Arithmetic)
 * ========================================================================= */
void
VR4300DSRA(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSRA.");
}

/* ============================================================================
 *  Instruction: DSRAV (Doubleword Shift Right Arithmetic Variable)
 * ========================================================================= */
void
VR4300DSRAV(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSRAV.");
}

/* ============================================================================
 *  Instruction: DSRA32 (Doubleword Shift Right Arithmetic + 32)
 * ========================================================================= */
void
VR4300DSRA32(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = (rfexLatch->iw >> 6 & 0x1F) + 32;
  unsigned dest = GET_RD(rfexLatch->iw);
  uint64_t result = (int64_t) rt >> sa;

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: DSRL (Doubleword Shift Right Logical)
 * ========================================================================= */
void
VR4300DSRL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rfexLatch->iw >> 6 & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  uint64_t result = rt >> sa;

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: DSRLV (Doubleword Shift Right Logical Variable)
 * ========================================================================= */
void
VR4300DSRLV(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSRLV.");
}

/* ============================================================================
 *  Instruction: DSRL32 (Doubleword Shift Right Logical + 32)
 * ========================================================================= */
void
VR4300DSRL32(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = (rfexLatch->iw >> 6 & 0x1F) + 32;
  unsigned dest = GET_RD(rfexLatch->iw);
  uint64_t result = rt >> sa;

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: DSUB (Doubleword Subtract)
 * ========================================================================= */
void
VR4300DSUB(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSUB.");
}

/* ============================================================================
 *  Instruction: DSUBU (Doubleword Subtract Unsigned)
 * ========================================================================= */
void
VR4300DSUBU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DSUBU.");
}

/* ==========================================================================
 *  Instruction: INV (Invalid Operation)
 * ========================================================================= */
void
VR4300INV(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
}

/* ============================================================================
 *  Instruction: J (Jump)
 * ========================================================================= */
void
VR4300J(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  uint64_t address = (rfexLatch->iw & 0x3FFFFFF) << 2;

#ifdef DO_FASTFORWARD
  uint32_t pcCheck = (icrfLatch->pc & 0x7FFFFFFF) - 4;
  uint32_t addressCheck = address;

  if (pcCheck == addressCheck && vr4300->pipeline.faultManager.killStage == -1)
    vr4300->pipeline.faultManager.killStage = VR4300_PIPELINE_STAGE_WB;
#endif

  icrfLatch->pc &= 0xFFFFFFFFF0000000ULL;
  icrfLatch->pc = (icrfLatch->pc | address) - 4;
}

/* ============================================================================
 *  Instruction: JAL (Jump And Link)
 * ========================================================================= */
void
VR4300JAL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  uint64_t address = (rfexLatch->iw & 0x3FFFFFF) << 2;

  exdcLatch->result.data = icrfLatch->pc + 4;
  exdcLatch->result.dest = VR4300_REGISTER_RA;

  icrfLatch->pc &= 0xFFFFFFFFF0000000ULL;
  icrfLatch->pc = (icrfLatch->pc | address) - 4;
}

/* ============================================================================
 *  Instruction: JALR (Jump And Link Register)
 * ========================================================================= */
void
VR4300JALR(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  exdcLatch->result.data = icrfLatch->pc + 4;
  exdcLatch->result.dest = VR4300_REGISTER_RA;

  icrfLatch->pc = rs - 4;
}

/* ============================================================================
 *  Instruction: JR (Jump Register)
 *  TODO: Throw exception if not word-aligned.
 * ========================================================================= */
void
VR4300JR(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  icrfLatch->pc = rs - 4;
}

/* ============================================================================
 *  Instruction: LB (Load Byte)
 * ========================================================================= */
void
VR4300LB(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadByte;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LBU (Load Byte Unsigned)
 * ========================================================================= */
void
VR4300LBU(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadByteU;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LD (Load Doubleword)
 * ========================================================================= */
void
VR4300LD(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x7)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadDWord;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LDC2 (Load Doubleword To Coprocessor 2)
 * ========================================================================= */
void
VR4300LDC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LDC2.");
}

/* ============================================================================
 *  Instruction: LDL (Load Doubleword Left)
 * ========================================================================= */
void
VR4300LDL(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadDWordLeft;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.data = rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LDR (Load Doubleword Right)
 * ========================================================================= */
void
VR4300LDR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadDWordRight;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.data = rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LH (Load Halfword)
 * ========================================================================= */
void
VR4300LH(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x1)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadHWord;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LHU (Load Halfword Unsigned)
 * ========================================================================= */
void
VR4300LHU(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x1)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadHWordU;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LL (Load Linked)
 * ========================================================================= */
void
VR4300LL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LL.");
}

/* ============================================================================
 *  Instruction: LLD (Load Linked Doubleword)
 * ========================================================================= */
void
VR4300LLD(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LLD.");
}

/* ============================================================================
 *  Instruction: LUI (Load Upper Immediate)
 * ========================================================================= */
void
VR4300LUI(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)){
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int32_t) (rfexLatch->iw << 16);

  exdcLatch->result.data = imm;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LW (Load Word)
 * ========================================================================= */
void
VR4300LW(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x3)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadWord;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LWC2 (Load Word To Coprocessor 2)
 * ========================================================================= */
void
VR4300LWC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LWC2.");
}

/* ============================================================================
 *  Instruction: LWL (Load Word Left)
 * ========================================================================= */
void
VR4300LWL(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadWordLeft;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.data = rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LWR (Load Word Right)
 * ========================================================================= */
void
VR4300LWR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadWordRight;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.data = rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: LWU (Load Word Unsigned)
 * ========================================================================= */
void
VR4300LWU(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x3)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadWordU;
  exdcLatch->memoryData.target = &dcwbLatch->result.data;

  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: MFC2 (Move From Coprocessor 2)
 * ========================================================================= */
void
VR4300MFC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: MFC2.");
}

/* ============================================================================
 *  Instruction: MFHI (Move From HI)
 * ========================================================================= */
void
VR4300MFHI(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = vr4300->regs[VR4300_REGISTER_HI];
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: MFLO (Move From LO)
 * ========================================================================= */
void
VR4300MFLO(struct VR4300 *vr4300,uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = vr4300->regs[VR4300_REGISTER_LO];
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: MTC2 (Move To Coprocessor 2)
 * ========================================================================= */
void
VR4300MTC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: MTC2.");
}

/* ============================================================================
 *  Instruction: MTHI (Move To HI)
 * ========================================================================= */
void
VR4300MTHI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  vr4300->regs[VR4300_REGISTER_HI] = rs;
}

/* ============================================================================
 *  Instruction: MTLO (Move To LO)
 * ========================================================================= */
void
VR4300MTLO(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  vr4300->regs[VR4300_REGISTER_LO] = rs;
}

/* ============================================================================
 *  Instruction: MULT (Multiply)
 *  TODO: Do we write LO/HI here, or wait until WB?
 * ========================================================================= */
void
VR4300MULT(struct VR4300 *vr4300, uint64_t _rs, uint64_t _rt) {
  int64_t rs = (int32_t) _rs;
  int64_t rt = (int32_t) _rt;
  int64_t result = rs * rt;

  int32_t lo = result;
  int32_t hi = result >> 32;

  vr4300->regs[VR4300_REGISTER_LO] = (int64_t) lo;
  vr4300->regs[VR4300_REGISTER_HI] = (int64_t) hi;
}

/* ============================================================================
 *  Instruction: MULTU (Multiply Unsigned)
 *  TODO: Do we write LO/HI here, or wait until WB?
 * ========================================================================= */
void
VR4300MULTU(struct VR4300 *vr4300, uint64_t _rs, uint64_t _rt) {
  uint64_t rs = (uint32_t) _rs;
  uint64_t rt = (uint32_t) _rt;
  uint64_t result = rs * rt;

  int32_t lo = result;
  int32_t hi = result >> 32;

  vr4300->regs[VR4300_REGISTER_LO] = (int64_t) lo;
  vr4300->regs[VR4300_REGISTER_HI] = (int64_t) hi;
}

/* ============================================================================
 *  Instruction: NOR (Nor)
 * ========================================================================= */
void
VR4300NOR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = ~(rs | rt);
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: OR (Or)
 * ========================================================================= */
void
VR4300OR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = rs | rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: ORI (Or Immediate)
 * ========================================================================= */
void
VR4300ORI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  uint16_t imm = rfexLatch->iw;

  exdcLatch->result.data = rs | imm;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SB (Store Byte)
 * ========================================================================= */
void
VR4300SB(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreByte;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SC (Store Conditional)
 * ========================================================================= */
void
VR4300SC(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SC.");
}

/* ============================================================================
 *  Instruction: SCD (Store Conditional Doubleword)
 * ========================================================================= */
void
VR4300SCD(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SCD.");
}

/* ============================================================================
 *  Instruction: SD (Store Doubleword)
 * ========================================================================= */
void
VR4300SD(struct VR4300 *vr4300,uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x7)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreDWord;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SDC2 (Store Doubleword From Coprocessor 2)
 * ========================================================================= */
void
VR4300SDC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SDC2.");
}

/* ============================================================================
 *  Instruction: SDL (Store Doubleword Left)
 * ========================================================================= */
void
VR4300SDL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SDL.");
}

/* ============================================================================
 *  Instruction: SDR (Store Doubleword Right)
 * ========================================================================= */
void
VR4300SDR(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SDR.");
}

/* ============================================================================
 *  Instruction: SH (Store Halfword)
 * ========================================================================= */
void
VR4300SH(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x1)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreHWord;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SLL (Shift Left Logical)
 * ========================================================================= */
void
VR4300SLL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rfexLatch->iw >> 6 & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rt << sa);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SLLV (Shift Left Logical Variable)
 * ========================================================================= */
void
VR4300SLLV(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rs & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rt << sa);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SLT (Set On Less Than)
 * ========================================================================= */
void
VR4300SLT(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = ((int64_t) rs < (int64_t) rt) ? 1 : 0;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SLTI (Set On Less Than Immediate)
 * ========================================================================= */
void
VR4300SLTI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;

  exdcLatch->result.data = ((int64_t) rs < imm) ? 1 : 0;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SLTIU (Set On Less Than Immediate Unsigned)
 * ========================================================================= */
void
VR4300SLTIU(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;

  exdcLatch->result.data = ((uint64_t) rs < (uint64_t) imm) ? 1 : 0;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SLTU (Set On Less Than Unsigned)
 * ========================================================================= */
void
VR4300SLTU(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = (rs < rt) ? 1 : 0;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SRA (Shift Right Arithmetic)
 * ========================================================================= */
void
VR4300SRA(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rfexLatch->iw >> 6 & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int32_t result = (int32_t) rt >> sa;

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SRAV (Shift Right Arithmetic Variable)
 * ========================================================================= */
void
VR4300SRAV(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rs & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int32_t result = (int32_t) rt >> sa;

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SRL (Shift Right Logical)
 * ========================================================================= */
void
VR4300SRL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rfexLatch->iw >> 6 & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int32_t result = (uint32_t) rt >> sa;

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SRLV (Shift Right Logical Variable)
 * ========================================================================= */
void
VR4300SRLV(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned sa = rs & 0x1F;
  unsigned dest = GET_RD(rfexLatch->iw);
  int32_t result = (uint32_t) rt >> sa;

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SUB (Subtract)
 *  TODO: Integer overflow exception.
 * ========================================================================= */
void
VR4300SUB(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rs - rt);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SUBU (Subtract Unsigned)
 * ========================================================================= */
void
VR4300SUBU(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);
  int64_t result = (int32_t) (rs - rt);

  exdcLatch->result.data = result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: SW (Store Word)
 * ========================================================================= */
void
VR4300SW(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (address & 0x3)
    debug("Unimplemented fault: VR4300_FAULT_DADE.");

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreWord;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SWC2 (Store Word From Coprocessor 2)
 * ========================================================================= */
void
VR4300SWC2(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SWC2.");
}

/* ============================================================================
 *  Instruction: SWL (Store Word Left)
 * ========================================================================= */
void
VR4300SWL(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreWordLeft;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SWR (Store Word Right)
 * ========================================================================= */
void
VR4300SWR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreWordRight;
  exdcLatch->memoryData.data = rt;
}

/* ============================================================================
 *  Instruction: SYNC (Synchronize)
 * ========================================================================= */
void
VR4300SYNC(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SYNC.");
}

/* ============================================================================
 *  Instruction: SYSCALL (System Call)
 * ========================================================================= */
void
VR4300SYSCALL(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SYSCALL.");
}

/* ============================================================================
 *  Instruction: TEQ (Trap If Equal)
 * ========================================================================= */
void
VR4300TEQ(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TEQ.");
}

/* ============================================================================
 *  Instruction: TEQI (Trap If Equal Immediate)
 * ========================================================================= */
void
VR4300TEQI(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TEQI.");
}

/* ============================================================================
 *  Instruction: TGE (Trap If Greater Than Or Equal)
 * ========================================================================= */
void
VR4300TGE(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TGE.");
}

/* ============================================================================
 *  Instruction: TGEI (Trap If Greater Than Or Equal Immediate)
 * ========================================================================= */
void
VR4300TGEI(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TGEI.");
}

/* ============================================================================
 *  Instruction: TGEIU (Trap If Greater Than Or Equal Immediate Unsigned)
 * ========================================================================= */
void
VR4300TGEIU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TGEIU.");
}

/* ============================================================================
 *  Instruction: TGEU (Trap If Greater Than Or Equal Unsigned)
 * ========================================================================= */
void
VR4300TGEU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TEGU.");
}

/* ============================================================================
 *  Instruction: TLT (Trap If Less Than) 
 * ========================================================================= */
void
VR4300TLT(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TLT.");
}

/* ============================================================================
 *  Instruction: TLTI (Trap If Less Than Immediate)
 * ========================================================================= */
void
VR4300TLTI(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TLTI.");
}

/* ============================================================================
 *  Instruction: TLTIU (Trap If Less Than Immediate Unsigned)
 * ========================================================================= */
void
VR4300TLTIU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TLTIU.");
}

/* ============================================================================
 *  Instruction: TLTU (Trap If Less Than Unsigned)
 * ========================================================================= */
void
VR4300TLTU(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TLTU.");
}

/* ============================================================================
 *  Instruction: TNE (Trap If Not Equal)
 * ========================================================================= */
void
VR4300TNE(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TNE.");
}

/* ============================================================================
 *  Instruction: TNEI (Trap If Not Equal Immediate)
 * ========================================================================= */
void
VR4300TNEI(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: TNEI.");
}

/* ============================================================================
 *  Instruction: XOR (Exclusive Or)
 * ========================================================================= */
void
VR4300XOR(struct VR4300 *vr4300, uint64_t rs, uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RD(rfexLatch->iw);

  exdcLatch->result.data = rs ^ rt;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: XORI (Exclusive Or Immediate)
 * ========================================================================= */
void
VR4300XORI(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned dest = GET_RT(rfexLatch->iw);
  uint16_t imm = rfexLatch->iw;

  exdcLatch->result.data = rs ^ imm;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  VR4300EXStage: Invokes the appropriate functional unit.
 * ========================================================================= */
void
VR4300EXStage(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  const struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  uint64_t rs, rt, temp = vr4300->regs[dcwbLatch->result.dest];
  unsigned rsForwardingRegister = GET_RS(rfexLatch->iw);
  unsigned rtForwardingRegister = GET_RT(rfexLatch->iw);

  /* Always invalidate results. */
  exdcLatch->result.dest = 0;

  /* Forward results from DC/WB into the register file (RF). */
  /* Copy/restore value to prevent the need for branches. */
  vr4300->regs[dcwbLatch->result.dest] = dcwbLatch->result.data;
  vr4300->regs[VR4300_REGISTER_ZERO] = 0;

  rs = vr4300->regs[rsForwardingRegister];
  rt = vr4300->regs[rtForwardingRegister];

  vr4300->regs[dcwbLatch->result.dest] = temp;

#ifndef NDEBUG
  VR4300OpcodeCounts[rfexLatch->opcode.id]++;
#endif

  /* Invoke the appropriate functional unit. */
  VR4300FunctionTable[rfexLatch->opcode.id](vr4300, rs, rt);
}

