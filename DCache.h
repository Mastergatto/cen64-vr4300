/* ============================================================================
 *  DCache.h: Data cache.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__DCACHE_H__
#define __VR4300__DCACHE_H__
#include "Common.h"
#include "Decoder.h"
#include "Externs.h"

struct VR4300DCacheLine {
  uint8_t data[16];
  uint32_t tag;
};

struct VR4300DCache {
  struct VR4300DCacheLine lines[512];
  bool valid[512];
};

void VR4300InitDCache(struct VR4300DCache *dcache);
void VR4300DCacheFill(struct VR4300DCache *dcache,
  struct BusController *bus, uint32_t paddr);

struct VR4300DCacheLine* VR4300DCacheProbe(
  struct VR4300DCache *dcache, uint32_t paddr);

#endif

