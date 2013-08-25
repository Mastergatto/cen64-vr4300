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
 *  Fills an instruction cache line, sets the tags, etc.
 * ========================================================================= */
void VR4300ICacheFill(struct VR4300ICache *icache,
  struct BusController *bus, uint32_t paddr) {
  struct VR4300ICacheLineData *data;
  unsigned lineIdx = paddr >> 5 & 0x1FF;
  unsigned ppo = paddr >> 5;
  unsigned i;

  /* Mark the line as valid. */
  data = icache->lines[lineIdx].data;
  icache->valid[lineIdx] = true;
  paddr &= 0xFFFFFFE0;

  /* And fill it entirely. */
  for (i = 0 ; i < 8; i++, paddr += 4, data++) {
    uint32_t word = BusReadWord(bus, paddr);
    data->opcode = *VR4300DecodeInstruction(word);
    data->word = word;
    data->tag = ppo;
  }
}

/* ============================================================================
 *  Probes the instruction cache using an address and tag.
 * ========================================================================= */
const struct VR4300ICacheLineData* VR4300ICacheProbe(
  const struct VR4300ICache *icache, uint32_t paddr) {
  const struct VR4300ICacheLineData *cacheData;
  unsigned lineIdx = paddr >> 5 & 0x1FF;
  unsigned offset = paddr >> 2 & 0x7;
  unsigned ppo = paddr >> 5;

  /* Virtually indexed, physically tagged. */
  cacheData = &icache->lines[lineIdx].data[offset];
  if (!icache->valid[lineIdx] || cacheData->tag != ppo)
    return NULL;

  return cacheData;
}

/* ============================================================================
 *  Initializes the instruction cache, invalidating all lines.
 * ========================================================================= */
void VR4300InitICache(struct VR4300ICache *icache) {
  memset(icache->valid, 0, sizeof(icache->valid));
}

