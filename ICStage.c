/* ============================================================================
 *  ICStage.c: Instruction cache fetch stage.
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
#include "Fault.h"
#include "Pipeline.h"
#include "Region.h"

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

/* ============================================================================
 *  VR4300ICStage: Fetches an instruction from ICache.
 * ========================================================================= */
void
VR4300ICStage(struct VR4300 *vr4300) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  uint64_t pc = icrfLatch->pc;

  /* TODO: Giant hack: bypass the ICache/ITLB/TLB for now. */
  if (pc < icrfLatch->region->start || pc > icrfLatch->region->end) {
    const struct RegionInfo *region;

    if ((region = GetRegionInfo(vr4300, pc)) == NULL) {
      QueueFault(&vr4300->pipeline.faultQueue, VR4300_FAULT_IADE);
      return;
    }

    icrfLatch->region = region;
  }

  icrfLatch->address = pc - icrfLatch->region->offset;
}

