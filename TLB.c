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
#include <cassert>
#include <cstring>
#else
#include <assert.h>
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
 *  TODO: Handle subtleties of 64-bit access.
 * ========================================================================= */
void
VR4300TLBP(struct VR4300 *vr4300) {
  vr4300->cp0.regs.index.probe = 0;

  /* Try to grab the node with the page start address. */
  const struct TLBNode* node = TLBTreeLookup(
    &vr4300->tlb.tlbTree, vr4300->cp0.regs.entryHi.asid,
    (uint64_t) vr4300->cp0.regs.entryHi.region << 62 |
    (uint64_t) vr4300->cp0.regs.entryHi.vpn2 << 13);

  if (node != NULL) {
    vr4300->cp0.regs.index.probe = 1;
    vr4300->cp0.regs.index.index = node - vr4300->tlb.tlbTree.entries;
  }
}

/* ==========================================================================
 *  Instruction: TLBR (Read Indexed TLB Entry)
 * ========================================================================= */
void
VR4300TLBR(struct VR4300 *vr4300) {
  unsigned idx = vr4300->cp0.regs.index.index;
  struct VR4300CP0* cp0 = &vr4300->cp0;

  const struct TLBNode* node = vr4300->tlb.tlbTree.entries + idx;

  memcpy(&cp0->regs.entryLo0, &node->tlbEntryLo0, sizeof(node->tlbEntryLo0));
  memcpy(&cp0->regs.entryLo1, &node->tlbEntryLo1, sizeof(node->tlbEntryLo1));
  memcpy(&cp0->regs.entryHi,  &node->tlbEntryHi,  sizeof(node->tlbEntryHi));
  cp0->regs.pageMask = node->pageMask;

  /* Strip the folded region out of VPN2. */
  cp0->regs.entryHi.vpn2 &= 0x7FFFFFF;
}

/* ==========================================================================
 *  Instruction: TLBWI (Write Indexed TLB Entry)
 * ========================================================================= */
void
VR4300TLBWI(struct VR4300 *vr4300) {
  struct EntryHi *entryHi = &vr4300->cp0.regs.entryHi;
  struct EntryLo *entryLo0 = &vr4300->cp0.regs.entryLo0;
  struct EntryLo *entryLo1 = &vr4300->cp0.regs.entryLo1;
  uint16_t pageMask = vr4300->cp0.regs.pageMask;
  struct TLBTree *tlbTree = &vr4300->tlb.tlbTree;
  struct TLBNode *node;

#ifndef NDEBUG
  debugarg("Mapping TLB Entry: %u", vr4300->cp0.regs.index.index);
  debugarg("TLB: ASID      : %u.", entryHi->asid);

  (entryLo0->global && entryLo1->global)
    ? debug("TLB: Global    : Yes.")
    : debug("TLB: Global    : No.");

  debugarg("TLB: PageMask  : [0x%.4x].", pageMask);

  debugarg("TLB: PPN0      : [0x%.8x].", entryLo0->pfn & pageMask); 
  debugarg("TLB: PPN1      : [0x%.8x].", entryLo1->pfn & pageMask); 

  (entryLo0->valid)
    ? debug("TLB: PPN0 Valid: Yes.")
    : debug("TLB: PPN0 Valid: No.");

  (entryLo1->valid)
    ? debug("TLB: PPN1 Valid: Yes.")
    : debug("TLB: PPN1 Valid: No.");

  debugarg("TLB: VPN2:  [0x%.8X].", entryHi->vpn2);
#endif

  /* Evict the old entry, setup up the new one, insert it. */
  node = tlbTree->entries + vr4300->cp0.regs.index.index;

  TLBTreeEvict(tlbTree, node);
  memcpy(&node->tlbEntryLo0, entryLo0, sizeof(*entryLo0));
  memcpy(&node->tlbEntryLo1, entryLo1, sizeof(*entryLo1));
  memcpy(&node->tlbEntryHi,  entryHi,  sizeof(*entryHi));
  node->pageMask = pageMask;
  TLBTreeInsert(tlbTree, node);
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

