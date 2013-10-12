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

#ifdef USE_SSE
#include <emmintrin.h>
#endif

/* ============================================================================
 *  ProduceLatchOutputs: Forces latch output to zero if iwMask == 0.
 * ========================================================================= */
static void
ProduceLatchOutputs(uint32_t iwMask,
  const struct VR4300ICacheLineData *cacheData,
  struct VR4300RFEXLatch *rfexLatch) {

#ifdef USE_SSE
    unsigned index = iwMask + 1;
    static const uint32_t maskTable[2][4] align(16) =
      {{~0, ~0, ~0, ~0},
       { 0,  0,  0,  0}};

    assert(sizeof(*cacheData) == 16);
    __m128i data = _mm_load_si128((__m128i*) cacheData);
    __m128i mask = _mm_load_si128((__m128i*) maskTable[index]);

    data = _mm_and_si128(data, mask);
    _mm_store_si128((__m128i*) &rfexLatch->opcode, data);
#else
    rfexLatch->iw = cacheData->word & iwMask;
    rfexLatch->opcode.id = cacheData->opcode.id & iwMask;
    rfexLatch->opcode.flags = cacheData->opcode.flags & iwMask;
#endif
}

/* ============================================================================
 *  VR4300RFStage: Decodes instructions, reads from cache, grabs registers.
 * ========================================================================= */
void
VR4300RFStage(struct VR4300 *vr4300) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  uint32_t address = icrfLatch->address;

  /* Always update PC. */
  rfexLatch->pc = icrfLatch->pc;
  icrfLatch->pc += 4;

  /* Is the region cache-able? */
  if (likely(icrfLatch->region->cached)) {
    const struct VR4300ICacheLineData *cacheData;
    cacheData = VR4300ICacheProbe(&vr4300->icache, address);

    /* Do we need to fill the line? */
    if (unlikely(cacheData == NULL)) {
      VR4300ICacheFill(&vr4300->icache, vr4300->bus, address);

      /* Now that it's cached, perform the probe again. */
      cacheData = VR4300ICacheProbe(&vr4300->icache, address);
    }

    ProduceLatchOutputs(icrfLatch->iwMask, cacheData, rfexLatch);
  }

  /* Region isn't cachable; fetch a word from memory. */
  /* Manually force instruction to invalid if iwMask == 0. */
  else {
    uint32_t iw = BusReadWord(vr4300->bus, address) & icrfLatch->iwMask;

    rfexLatch->opcode = *VR4300DecodeInstruction(iw);
    rfexLatch->opcode.id &= icrfLatch->iwMask;
    rfexLatch->iw = iw;
  }

  /* Reset the mask. */
  icrfLatch->iwMask = ~0;
}

