/* ============================================================================
 *  CP0.h: VR4300 Coprocessor #0.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CP0.h"
#include "CPU.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

/* ============================================================================
 *  Instruction: BC0 (Branch on Coprocessor 0 Instructions)
 * ========================================================================= */
void
VR4300BC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: BC0.");
}

/* ============================================================================
 *  Instruction: CFC0 (Move Control From Coprocessor 0)
 * ========================================================================= */
void
VR4300CFC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: CFC0.");
}

/* ============================================================================
 *  Instruction: COP0 (Coprocessor 0 Instructions)
 * ========================================================================= */
void
VR4300COP0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: COP0.");
}

/* ============================================================================
 *  Instruction: CTC0 (Move Control to Coprocessor 0)
 * ========================================================================= */
void
VR4300CTC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: CTC0.");
}

/* ============================================================================
 *  Instruction: DMFC0 (Doubleword Move From Coprocessor 0)
 * ========================================================================= */
void
VR4300DMFC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMFC0.");
}

/* ============================================================================
 *  Instruction: DMTC0 (Doubleword Move To Coprocessor 0)
 * ========================================================================= */
void
VR4300DMTC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: DMTC0.");
}

/* ==========================================================================
 *  Instruction: ERET (Return From Exception)
 * ========================================================================= */
void
VR4300ERET(struct VR4300 *vr4300) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;

  if (vr4300->cp0.regs.status.erl) {
    debugarg("ERET: Returning to [0x%.16lX].", vr4300->cp0.regs.errorEPC);
    vr4300->pipeline.icrfLatch.pc = vr4300->cp0.regs.errorEPC - 4;
    vr4300->cp0.regs.status.erl = 0;
  }

  else {
    debugarg("ERET: Returning to [0x%.16lX].", vr4300->cp0.regs.epc);
    vr4300->pipeline.icrfLatch.pc = vr4300->cp0.regs.epc - 4;
    vr4300->cp0.regs.status.exl = 0;
  }

  /* Flatten branches in main loop. */
  static const uint8_t mask[2] = {0, 0xFF};

  vr4300->cp0.interruptRaiseMask = mask[
    vr4300->cp0.regs.status.ie &&
    !vr4300->cp0.regs.status.exl &&
    !vr4300->cp0.regs.status.erl];

  vr4300->cp0.regs.llBit = 0;
  icrfLatch->iwMask = 0;
}

/* ============================================================================
 *  Instruction: LDC0 (Load Doubleword To Coprocessor 0)
 * ========================================================================= */
void
VR4300LDC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LDC0.");
}

/* ============================================================================
 *  Instruction: LWC0 (Load Word To Coprocessor 0)
 * ========================================================================= */
void
VR4300LWC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: LWC0.");
}

/* ============================================================================
 *  Instruction: MFC0 (Move From Coprocessor 0)
 * ========================================================================= */
void
VR4300MFC0(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned rd = rfexLatch->iw >> 11 & 0x1F;
  unsigned dest = GET_RT(rfexLatch->iw);
  int32_t result;

  switch((enum VR4300CP0RegisterID) rd) {
  case VR4300_CP0_REGISTER_INDEX:
    result =
      (uint32_t) vr4300->cp0.regs.index.index |
      (uint32_t) vr4300->cp0.regs.index.probe << 31;
    break;

  case VR4300_CP0_REGISTER_COUNT:
    result = vr4300->cp0.regs.count;
    break;

  case VR4300_CP0_REGISTER_ENTRYHI:
    result =
      (uint64_t) vr4300->cp0.regs.entryHi.asid |
      (uint64_t) vr4300->cp0.regs.entryHi.vpn2 << 13 |
      (uint64_t) vr4300->cp0.regs.entryHi.region << 62;
    break;

  case VR4300_CP0_REGISTER_COMPARE:
    result = vr4300->cp0.regs.compare;
    break;

  case VR4300_CP0_REGISTER_STATUS:
    result =
      (uint32_t) vr4300->cp0.regs.status.ie |
      (uint32_t) vr4300->cp0.regs.status.exl << 1 |
      (uint32_t) vr4300->cp0.regs.status.erl << 2 |
      (uint32_t) vr4300->cp0.regs.status.ksu << 3 |
      (uint32_t) vr4300->cp0.regs.status.ux << 5 |
      (uint32_t) vr4300->cp0.regs.status.sx << 6 |
      (uint32_t) vr4300->cp0.regs.status.kx << 7 |
      (uint32_t) vr4300->cp0.regs.status.im << 8 |
      (uint32_t) vr4300->cp0.regs.status.ds.de << 16 |
      (uint32_t) vr4300->cp0.regs.status.ds.ce << 17 |
      (uint32_t) vr4300->cp0.regs.status.ds.ch << 18 |
      (uint32_t) vr4300->cp0.regs.status.ds.sr << 20 |
      (uint32_t) vr4300->cp0.regs.status.ds.ts << 21 |
      (uint32_t) vr4300->cp0.regs.status.ds.bev << 22 |
      (uint32_t) vr4300->cp0.regs.status.ds.its << 24 |
      (uint32_t) vr4300->cp0.regs.status.re << 25 |
      (uint32_t) vr4300->cp0.regs.status.fr << 26 |
      (uint32_t) vr4300->cp0.regs.status.rp << 27 |
      (uint32_t) vr4300->cp0.regs.status.cu << 28;
    break;

  case VR4300_CP0_REGISTER_CAUSE:
    result =
      (uint32_t) vr4300->cp0.regs.cause.excCode << 2 |
      (uint32_t) vr4300->cp0.regs.cause.ip << 8 |
      (uint32_t) vr4300->cp0.regs.cause.ce << 28 |
      (uint32_t) vr4300->cp0.regs.cause.bd << 31;
    break;

  case VR4300_CP0_REGISTER_EPC:
    result = vr4300->cp0.regs.epc;
    break;

  default:
    debugarg("Unimplemented function: MFC0 [rd = %u].", rd);

    result = 0;
    dest = 0;
  }

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = dest;
}

/* ============================================================================
 *  Instruction: MTC0 (Move To Coprocessor 0)
 *  TODO: Add delays to CP0 writes.
 * ========================================================================= */
void
VR4300MTC0(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  unsigned rd = rfexLatch->iw >> 11 & 0x1F;

  switch((enum VR4300CP0RegisterID) rd) {
  case VR4300_CP0_REGISTER_INDEX:
    /* TODO: Do we clear the probe bit here? */
    vr4300->cp0.regs.index.index = rt & 0x3F;
    break;

  case VR4300_CP0_REGISTER_ENTRYLO0:
    vr4300->cp0.regs.entryLo0.global = rt & 0x1;
    vr4300->cp0.regs.entryLo0.valid = rt >> 1 & 0x1;
    vr4300->cp0.regs.entryLo0.dirty = rt >> 2 & 0x1;
    vr4300->cp0.regs.entryLo0.attribute = rt >> 3 & 0x7;
    vr4300->cp0.regs.entryLo0.pfn = rt >> 6 & 0xFFFFF;
    break;

  case VR4300_CP0_REGISTER_ENTRYLO1:
    vr4300->cp0.regs.entryLo1.global = rt & 0x1;
    vr4300->cp0.regs.entryLo1.valid = rt >> 1 & 0x1;
    vr4300->cp0.regs.entryLo1.dirty = rt >> 2 & 0x1;
    vr4300->cp0.regs.entryLo1.attribute = rt >> 3 & 0x7;
    vr4300->cp0.regs.entryLo1.pfn = rt >> 6 & 0xFFFFF;
    break;

  case VR4300_CP0_REGISTER_PAGEMASK:
    vr4300->cp0.regs.pageMask = rt >> 13 & 0xFFF;
    break;

  case VR4300_CP0_REGISTER_COUNT:
    vr4300->cp0.regs.count = rt;
    break;

  case VR4300_CP0_REGISTER_ENTRYHI:
    vr4300->cp0.regs.entryHi.asid = rt & 0xFF;
    vr4300->cp0.regs.entryHi.vpn2 = rt >> 13 & 0x7FFFFFF;
    vr4300->cp0.regs.entryHi.region = rt >> 62 & 0x3;
    break;

  case VR4300_CP0_REGISTER_COMPARE:
    vr4300->cp0.regs.cause.ip &= ~0x80;
    vr4300->cp0.regs.compare = rt;
    break;

  case VR4300_CP0_REGISTER_STATUS:
    vr4300->cp0.regs.status.ie = rt & 0x1;
    vr4300->cp0.regs.status.exl = rt >> 1 & 0x1;
    vr4300->cp0.regs.status.erl = rt >> 2 & 0x1;
    vr4300->cp0.regs.status.ksu = rt >> 3 & 0x3;
    vr4300->cp0.regs.status.ux = rt >> 5 & 0x1;
    vr4300->cp0.regs.status.sx = rt >> 6 & 0x1;
    vr4300->cp0.regs.status.kx = rt >> 7 & 0x1;
    vr4300->cp0.regs.status.im = rt >> 8 & 0xFF;
    vr4300->cp0.regs.status.ds.de = rt >> 16 & 0x1;
    vr4300->cp0.regs.status.ds.ce = rt >> 17 & 0x1;
    vr4300->cp0.regs.status.ds.ch = rt >> 18 & 0x1;
    vr4300->cp0.regs.status.ds.sr = rt >> 20 & 0x1;
    vr4300->cp0.regs.status.ds.bev = rt >> 22 & 0x1;
    vr4300->cp0.regs.status.ds.its = rt >> 24 & 0x1;
    vr4300->cp0.regs.status.re = rt >> 25 & 0x1;
    vr4300->cp0.regs.status.fr = rt >> 26 & 0x1;
    vr4300->cp0.regs.status.rp = rt >> 27 & 0x1;
    vr4300->cp0.regs.status.cu = rt >> 28 & 0xF;
    assert(vr4300->cp0.regs.status.re == 0);

    /* Flatten branches in main loop. */
    static const uint8_t mask[2] = {0, 0xFF};

    vr4300->cp0.interruptRaiseMask = mask[
      vr4300->cp0.regs.status.ie &&
      !vr4300->cp0.regs.status.exl &&
      !vr4300->cp0.regs.status.erl
    ];

    break;

  case VR4300_CP0_REGISTER_CAUSE:
    vr4300->cp0.regs.cause.ip &= ~0x03;
    vr4300->cp0.regs.cause.ip |= rt >> 8 & 0x3;
    assert((vr4300->cp0.regs.cause.ip & 0x3) == 0);
    break;

  case VR4300_CP0_REGISTER_EPC:
    vr4300->cp0.regs.epc = rt;
    break;

  case VR4300_CP0_REGISTER_CONFIG:
    vr4300->cp0.regs.config.k0 = rt & 0x7;
    vr4300->cp0.regs.config.cu = rt >> 3 & 0x1;
    vr4300->cp0.regs.config.be = rt >> 15 & 0x1;
    vr4300->cp0.regs.config.ep = rt >> 24 & 0xF;
    break;

  case VR4300_CP0_REGISTER_TAGLO:
    vr4300->cp0.regs.tagLo.pState = rt >> 6 & 0x3;
    vr4300->cp0.regs.tagLo.pTagLo = rt >> 8 & 0xFFFFF;

  case VR4300_CP0_REGISTER_TAGHI:
    break;

  default:
    debugarg("Unimplemented function: MTC0 [rd = %u].", rd);
  }
}

/* ============================================================================
 *  Instruction: SDC0 (Store Doubleword From Coprocessor 0)
 * ========================================================================= */
void
VR4300SDC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SDC0.");
}

/* ============================================================================
 *  Instruction: SWC0 (Store Word From Coprocessor 0)
 * ========================================================================= */
void
VR4300SWC0(struct VR4300 *unused(vr4300),
  uint64_t unused(rs), uint64_t unused(rt)) {
  debug("Unimplemented function: SWC0.");
}

/* ============================================================================
 *  VR4300InitCP0: Initializes the co-processor.
 * ========================================================================= */
void
VR4300InitCP0(struct VR4300CP0 *cp0) {
  debug("Initializing CP0.");
  memset(cp0, 0, sizeof(*cp0));
}

