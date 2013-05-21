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
  unsigned i;

  /* Mark the line as valid. */
  icache->valid[lineIdx] = true;

  /* And fill it entirely. */
  for (i = 0 ; i < 8; i++) {
    struct VR4300ICacheLineData *data = &icache->lines[lineIdx].data[i];
    uint32_t address = paddr + i * 4;

    uint32_t word = BusReadWord(bus, address);
    data->opcode = *VR4300DecodeInstruction(word);
    data->tag = address;
    data->word = word;
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

  /* Virtually indexed, physically tagged. */
  cacheData = &icache->lines[lineIdx].data[offset];
  if (!icache->valid[lineIdx] || cacheData->tag != paddr)
    return NULL;

  return cacheData;
}

