/* ============================================================================
 *  ICache.h: Instruction Cache.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__ICACHE_H__
#define __VR4300__ICACHE_H__
#include "Common.h"
#include "Decoder.h"
#include "Externs.h"

struct VR4300ICacheLineData {
  struct VR4300Opcode opcode;
  uint32_t word, padding;
};

struct VR4300ICacheLine {
  struct VR4300ICacheLineData data[8];
  uint32_t tag;
};

struct VR4300ICache {
  struct VR4300ICacheLine lines[512];
  bool valid[512];
};

void VR4300InitICache(struct VR4300ICache *);

void VR4300ICacheFill(struct VR4300ICache *,
  struct BusController *, uint32_t);
const struct VR4300ICacheLineData* VR4300ICacheProbe(
  const struct VR4300ICache *, uint32_t);

#endif

