/* ============================================================================
 *  RFStage.c: Register fetch stage.
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
#include "Externs.h"
#include "ICache.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstring>
#else
#include <stddef.h>
#include <string.h>
#endif

/* ============================================================================
 *  VR4300RFStage: Decodes instructions, reads from cache, grabs registers.
 * ========================================================================= */
void
VR4300RFStage(struct VR4300 *vr4300) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  uint32_t address = icrfLatch->address;
  uint32_t iw;

  /* Always update PC. */
  rfexLatch->pc = icrfLatch->pc;
  icrfLatch->pc += 4;

  /* Is the region cache-able? */
  if (likely(icrfLatch->region->cached)) {
    const struct VR4300ICacheLineData *cacheData;
    cacheData = VR4300ICacheProbe(&vr4300->icache, address);

    /* Do we need to fill the line? */
    if (unlikely(cacheData == NULL)) {
      uint32_t lineAddress = address & 0xFFFFFFFE0;
      VR4300ICacheFill(&vr4300->icache, vr4300->bus, lineAddress);
      vr4300->pipeline.stalls = 100; /* TODO: Hack. */

      /* Now that it's cached, perform the probe again. */
      cacheData = VR4300ICacheProbe(&vr4300->icache, address);
    }

    rfexLatch->opcode.id = cacheData->opcode.id & icrfLatch->iwMask;
    rfexLatch->opcode.flags = cacheData->opcode.flags;
    rfexLatch->iw = cacheData->word;
  }

  else {
    iw = BusReadWord(vr4300->bus, address) & icrfLatch->iwMask;
    rfexLatch->opcode = *VR4300DecodeInstruction(iw);
    rfexLatch->iw = iw;

    vr4300->pipeline.stalls = 100; /* TODO: Hack. */
  }

  /* Reset the mask. */
  icrfLatch->iwMask = ~0;
}

