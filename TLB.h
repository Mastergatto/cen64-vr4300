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
#ifndef __VR4300__TLB_H__
#define __VR4300__TLB_H__
#include "Common.h"
#include "TLB.h"
#include "TLBTree.h"

struct VR4300TLB {
  struct TLBTree tlbTree;
};

void VR4300InitTLB(struct VR4300TLB *);

#endif

