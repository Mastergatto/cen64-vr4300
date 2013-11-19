/* ============================================================================
 *  DCache.c: Data cache.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "DCache.h"
#include "Externs.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstring>
#else
#include <stddef.h>
#include <string.h>
#endif

/* ============================================================================
 *  Returns the data cache line and sets the tags.
 * ========================================================================= */
void VR4300DCacheFill(struct VR4300DCache *dcache, uint32_t paddr) {
  unsigned lineIdx = paddr >> 4 & 0x1FF;
  unsigned ppo = paddr >> 4;
  unsigned i;

  /* Mark the line as valid. */
  struct VR4300DCacheLine *line = dcache->lines + lineIdx;
  dcache->valid[lineIdx] = true;
  line->tag = ppo;

  /* And fill it entirely. */
  paddr &= 0xFFFFFFF0;

  for (i = paddr ; i < (paddr + 16); i += 4) {
    uint32_t word = BusReadWord(dcache->bus, i);
    memcpy(line->data + i, &word, sizeof(word));
  }
}

/* ============================================================================
 *  Probes the data cache using an address.
 * ========================================================================= */
struct VR4300DCacheLine* VR4300DCacheProbe(
  struct VR4300DCache *dcache, uint32_t paddr) {
  struct VR4300DCacheLine *line;
  unsigned lineIdx = paddr >> 4 & 0x1FF;
  unsigned ppo = paddr >> 4;

  /* Virtually indexed, physically tagged. */
  line = dcache->lines + lineIdx;
  if (!dcache->valid[lineIdx] || line->tag != ppo)
    return NULL;

  return line;
}

/* ============================================================================
 *  Initializes the instruction cache, invalidating all lines.
 * ========================================================================= */
void VR4300InitDCache(struct VR4300DCache *dcache) {
  memset(dcache->valid, 0, sizeof(dcache->valid));
  dcache->bus = NULL;
}

