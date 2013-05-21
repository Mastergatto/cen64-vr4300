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
#include <cassert>
#include <cstddef>
#include <cstring>
#else
#include <assert.h>
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
  uint64_t pc = icrfLatch->pc;
  uint32_t iw;

  /* Always update PC. */
  icrfLatch->pc += 4;

  /* If it's a cached region... */
  if (icrfLatch->region->cached) {
    const struct VR4300ICacheLineData *cacheData;

    /* Perform a lookup in the ICache, and fill the line as needed. */
    if ((cacheData = VR4300ICacheProbe(&vr4300->icache, address)) == NULL) {
      VR4300ICacheFill(&vr4300->icache, vr4300->bus, address & 0xFFFFFFE0);
      vr4300->pipeline.stalls = 100; /* TODO: Hack. */

      /* Now that it's cached, perform the probe again. */
      cacheData = VR4300ICacheProbe(&vr4300->icache, address);
    }

    iw = cacheData->word & icrfLatch->iwMask;
  }

  else {
    iw = BusReadWord(vr4300->bus, address) & icrfLatch->iwMask;
    vr4300->pipeline.stalls = 100; /* TODO: Hack. */
  }

  /* Reset the mask again. */
  icrfLatch->iwMask = ~0;

  /* Decode the instruction, save the results. */
  rfexLatch->opcode = *VR4300DecodeInstruction(iw);
  rfexLatch->pc = pc;
  rfexLatch->iw = iw;
}

