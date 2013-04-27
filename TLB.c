/* ============================================================================
 *  TLB.h: Translation lookaside buffer.
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
#include "Pipeline.h"
#include "TLB.h"
#include "TLBTree.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

void VR4300ERET(struct VR4300 *);

/* ============================================================================
 *  InitVR4300: Initializes the VR4300.
 * ========================================================================= */
void
VR4300InitTLB(struct VR4300TLB *tlb) {
  debug("Initializing TLB.");
  InitTLBTree(&tlb->tlbTree);
}

/* ==========================================================================
 *  Instruction: TLBINV (Invalid TLB Operation)
 * ========================================================================= */
void
VR4300TLBINV(struct VR4300 *unused(vr4300)) {
  debug("Invalid TLB operation.");
}

/* ==========================================================================
 *  Instruction: TLBP (Probe TLB For Matching Entry)
 * ========================================================================= */
void
VR4300TLBP(struct VR4300 *vr4300) {
  debug("Unimplemented function: TLBP.");
  vr4300->cp0.regs.index.probe = 1;
}

/* ==========================================================================
 *  Instruction: TLBR (Read Indexed TLB Entry)
 * ========================================================================= */
void
VR4300TLBR(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: TLBR.");
}

/* ==========================================================================
 *  Instruction: TLBWI (Write Indexed TLB Entry)
 * ========================================================================= */
void
VR4300TLBWI(struct VR4300 *unused(vr4300)) {
#if 0
  struct EntryHi *entryHi = &vr4300->cp0.regs.entryHi;
  struct EntryLo *entryLo0 = &vr4300->cp0.regs.entryLo0;
  struct EntryLo *entryLo1 = &vr4300->cp0.regs.entryLo1;
  uint16_t pageMask = vr4300->cp0.regs.pageMask;

  debugarg("Mapping TLB Entry: %u", vr4300->cp0.regs.index.index);
  debugarg("ASID      : %u.", entryHi->asid);

  (entryLo0->global && entryLo1->global)
    ? debug("Global    : Yes.")
    : debug("Global    : No.");

  debugarg("PageMask  : [0x%.4x].", pageMask);

  debugarg("PPN0      : [0x%.8x].", entryLo0->pfn & pageMask); 
  debugarg("PPN1      : [0x%.8x].", entryLo1->pfn & pageMask); 

  (entryLo0->valid)
    ? debug("PPN0 Valid: Yes.")
    : debug("PPN0 Valid: No.");

  (entryLo1->valid)
    ? debug("PPN1 Valid: Yes.")
    : debug("PPN1 Valid: No.");

  debugarg("VPN:  [0x%.16lx].", (uint64_t) entryHi->vpn << 13);
#endif
}

/* ==========================================================================
 *  Instruction: TLBWR (Write Random TLB Entry)
 * ========================================================================= */
void
VR4300TLBWR(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: TLBWR.");
}

/* ============================================================================
 *  Callback table.
 *
 *      31--------26-25------21 -------------------------------5--------0
 *      |  COP0/6   |  TLB/5  |                               |  FMT/6  |
 *      ------6----------5-----------------------------------------6-----
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  |
 *  001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
typedef void (*const VR4300TLBFunction)(struct VR4300 *);
static const VR4300TLBFunction VR4300TLBFunctionTable[64] = {
  &VR4300TLBINV, &VR4300TLBR,   &VR4300TLBWI,  &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBWR,  &VR4300TLBINV,
  &VR4300TLBP,   &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300ERET,   &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
  &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV, &VR4300TLBINV,
};

/* ============================================================================
 *  Instruction: TLB (Coprocessor 0 TLB Instructions)
 * ========================================================================= */
void
VR4300TLB(struct VR4300 *vr4300,uint64_t unused(rs), uint64_t unused(rt)) {
  VR4300TLBFunctionTable[vr4300->pipeline.rfexLatch.iw & 0x3F](vr4300);
}

