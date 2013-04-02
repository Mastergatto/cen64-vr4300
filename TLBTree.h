/* ============================================================================
 *  TLBTree.h: TLB backing structure.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__TLBTREE_H__
#define __VR4300__TLBTREE_H__
#define NUM_TLB_ENTRIES 32
#include "Common.h"
#include "CP0.h"

enum TLBTreeColor {
  TLBTREE_BLACK,
  TLBTREE_RED,
};

struct TLBNode {
  struct TLBNode *left;
  struct TLBNode *parent;
  struct TLBNode *right;

  struct EntryLo tlbEntryLo0;
  struct EntryLo tlbEntryLo1;
  struct EntryHi tlbEntryHi;
  uint16_t pageMask;

  enum TLBTreeColor color;
  unsigned long long hits;
};

struct TLBTree {
  struct TLBNode entries[NUM_TLB_ENTRIES];

  struct TLBNode *globalEntryRoot;
  struct TLBNode *asidEntryRoot;
  struct TLBNode globalNilNode;
  struct TLBNode asidNilNode;
};

void InitTLBTree(struct TLBTree *);

void TLBTreeEvict(struct TLBTree *, struct TLBNode *);
int TLBTreeInsert(struct TLBTree *, struct TLBNode *);
const struct TLBNode* TLBTreeLookup(const struct TLBTree *,
  uint8_t, uint64_t);

#endif

