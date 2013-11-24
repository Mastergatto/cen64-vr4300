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
void VR4300DCacheFill(struct VR4300DCache *dcache,
  struct BusController *bus, uint32_t paddr) {
  unsigned lineIdx = paddr >> 4 & 0x1FF;
  unsigned ppo = paddr >> 4;
  unsigned i;

  struct VR4300DCacheLine *line = dcache->lines + lineIdx;
  paddr &= 0xFFFFFFF0;

  /* If the line is currently valid, flush it out. */
  if (dcache->valid[lineIdx]) {
    MemoryFunction write;
    uint32_t wraddr;
    void *opaque;

    wraddr = line->tag << 4;
    write = BusWrite(bus, BUS_TYPE_WORD, wraddr, &opaque);

    for (i = 0; i < 16; i += 4) {
      uint32_t word;
      memcpy(&word, line->data + i, sizeof(word));
      word = ByteOrderSwap32(word);
      write(opaque, wraddr + i, &word);
    }
  }

  /* Mark the line as valid. */
  dcache->valid[lineIdx] = true;
  line->tag = ppo;

  /* And fill it entirely. */
  for (i = 0 ; i < 16; i += 4) {
    uint32_t word = ByteOrderSwap32(BusReadWord(bus, paddr + i));
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
}

