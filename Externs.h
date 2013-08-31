/* ============================================================================
 *  Externs.h: External definitions for the VR4300 plugin.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__EXTERNS_H__
#define __VR4300__EXTERNS_H__
#include <stddef.h>

struct BusController;

enum BusType {
  BUS_TYPE_BYTE = 0,
  BUS_TYPE_HWORD = 1,
  BUS_TYPE_WORD = 2,
  BUS_TYPE_UWORD = 3,
  BUS_TYPE_UDWORD = 3,
  BUS_TYPE_DWORD = 4,
};

/* When writing unaligned data, use this structure. */
/* The size can be any value between 1 <= x <= 8 bytes. */
struct UnalignedData {
  uint8_t data[8];
  size_t size;
};

typedef int (*MemoryFunction)(void *opaque,
  uint32_t address, void *contents);

MemoryFunction BusRead(const struct BusController *bus,
  unsigned type, uint32_t address, void **opaque);
MemoryFunction BusWrite(const struct BusController *bus,
  unsigned type, uint32_t address, void **opaque);

/* This "old" memory read function is reserved for IW reads. */
uint32_t BusReadWord(const struct BusController *, uint32_t);

#endif

