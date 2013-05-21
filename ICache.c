/* ============================================================================
 *  ICache.c: Instruction Cache.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "Decoder.h"
#include "Externs.h"
#include "ICache.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstring>
#else
#include <stddef.h>
#include <string.h>
#endif

/* ============================================================================
 *  Initializes the instruction cache, invalidating all lines.
 * ========================================================================= */
void VR4300InitICache(struct VR4300ICache *icache) {
  memset(icache->valid, 0, sizeof(icache->valid));
}

/* ============================================================================
 *  Fills an instruction cache line, sets the tags, etc.
 * ========================================================================= */
void VR4300ICacheFill(struct VR4300ICache *icache,
  struct BusController *bus, uint32_t paddr) {
  unsigned lineIdx = paddr >> 5 & 0x1FF;
  uint32_t tag = paddr >> 12;
  unsigned i;

  /* Mark the line as valid. */
  icache->valid[lineIdx] = true;
  icache->tags[lineIdx] = tag;

  /* And fill it entirely. */
  for (i = 0 ; i < 8; i++) {
    uint32_t word = BusReadWord(bus, paddr + i * 4);
    icache->lines[lineIdx].data[i].word = word;
  }
}

/* ============================================================================
 *  Probes the instruction cache using an address and tag.
 * ========================================================================= */
const struct VR4300ICacheLineData* VR4300ICacheProbe(
  const struct VR4300ICache *icache, uint32_t paddr) {
  unsigned lineIdx = paddr >> 5 & 0x1FF;
  unsigned offset = paddr >> 2 & 0x7;
  uint32_t tag = paddr >> 12;

  /* Virtually indexed, physically tagged. */
  if (!icache->valid[lineIdx] || icache->tags[lineIdx] != tag)
    return NULL;

  return &icache->lines[lineIdx].data[offset];
}

