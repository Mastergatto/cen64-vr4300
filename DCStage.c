/* ============================================================================
 *  DCStage.c: Data cache fetch stage.
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
#include "DCStage.h"
#include "Fault.h"
#include "Pipeline.h"
#include "Region.h"

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
 *  VR4300DCStage: Reads or writes data from or to DCache/Bus.
 * ========================================================================= */
void
VR4300DCStage(struct VR4300 *vr4300) {
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;
  struct VR4300MemoryData *memoryData = &exdcLatch->memoryData;
  VR4300MemoryFunction function = memoryData->function;
  uint64_t address = memoryData->address;

  if (function == NULL) {
    dcwbLatch->result = exdcLatch->result;
    return;
  }

  /* Reset before EXStage. */
  memoryData->function = NULL;

  /* Check if the address lies in the cache region, or look it up. */
  if (address < dcwbLatch->region->start || address > dcwbLatch->region->end) {
    const struct RegionInfo *region;

    if ((region = GetRegionInfo(vr4300, address)) == NULL) {
      QueueFault(&vr4300->pipeline.faultManager, VR4300_FAULT_DADE);
      memset(&dcwbLatch->result, 0, sizeof(dcwbLatch->result));
      return;
    }

    dcwbLatch->region = region;
  }

  memoryData->address -= dcwbLatch->region->offset;
  function(memoryData, vr4300->bus);
  dcwbLatch->result = exdcLatch->result;
}

/* ============================================================================
 *  VR4300LoadByte: Reads a byte from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadByte(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  int64_t *dest = (int64_t*) memoryData->target;
  *dest = (int8_t) BusReadByte(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadByteU: Reads a byte from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadByteU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  *dest = BusReadByte(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadDWord: Reads a doubleword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadDWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  *dest = (uint64_t) BusReadDWord(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadDWordLeft: Reads a doubleword from the DCache/Bus.
 * ========================================================================= */
static const uint64_t LoadDWordLeftMaskTable[8] = {
  0x0000000000000000ULL,
  0x00000000000000FFULL,
  0x000000000000FFFFULL,
  0x0000000000FFFFFFULL,
  0x00000000FFFFFFFFULL,
  0x000000FFFFFFFFFFULL,
  0x0000FFFFFFFFFFFFULL,
  0x00FFFFFFFFFFFFFFULL,
};

void
VR4300LoadDWordLeft(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  unsigned type = memoryData->address & 0x7;
  uint64_t dword;

  dword = BusReadDWord(bus, memoryData->address & 0xFFFFFFF8) << (type * 8);
  dword |= (*dest & LoadDWordLeftMaskTable[type]);
  *dest = dword;
}

/* ============================================================================
 *  VR4300LoadDWordRight: Reads a doubleword from the DCache/Bus.
 * ========================================================================= */
static const uint64_t LoadDWordRightMaskTable[8] = {
  0x00000000000000FFULL,
  0x000000000000FFFFULL,
  0x0000000000FFFFFFULL,
  0x00000000FFFFFFFFULL,
  0x000000FFFFFFFFFFULL,
  0x0000FFFFFFFFFFFFULL,
  0x00FFFFFFFFFFFFFFULL,
  0xFFFFFFFFFFFFFFFFULL,
};

void
VR4300LoadDWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  unsigned type = memoryData->address & 0x7;
  size_t size = type + 1;
  uint64_t dword;

  dword = BusReadDWord(bus, memoryData->address & 0xFFFFFFF8) >> (8 - size) * 8;
  dword |= (uint64_t) (*dest & LoadDWordRightMaskTable[type]);
  *dest = dword;
}

/* ============================================================================
 *  VR4300LoadHWord: Reads a halfword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadHWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  int64_t *dest = (int64_t*) memoryData->target;
  *dest = (int16_t) BusReadHWord(bus, memoryData->address);
}
/* ============================================================================
 *  VR4300LoadHWordU: Reads a halfword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadHWordU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  *dest = BusReadHWord(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadWord: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  int64_t *dest = (int64_t*) memoryData->target;
  *dest = (int32_t) BusReadWord(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadDWordFPU: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadDWordFPU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint64_t *dest = (uint64_t*) memoryData->target;
  *dest = BusReadDWord(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadWordFPU: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadWordFPU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  uint32_t *dest = (uint32_t*) memoryData->target;
  *dest = BusReadWord(bus, memoryData->address);
}

/* ============================================================================
 *  VR4300LoadWordLeft: Reads a word from the DCache/Bus.
 * ========================================================================= */
static const uint32_t LoadWordLeftMaskTable[4] = {
  0x00000000,
  0x000000FF,
  0x0000FFFF,
  0x00FFFFFF
};

void
VR4300LoadWordLeft(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  int64_t *dest = (int64_t*) memoryData->target;
  unsigned type = memoryData->address & 0x3;
  int32_t word;

  word = BusReadWord(bus, memoryData->address & 0xFFFFFFFC) << (type * 8);
  word |= (uint32_t) (*dest & LoadWordLeftMaskTable[type]);
  *dest = (int64_t) word;
}

/* ============================================================================
 *  VR4300LoadWordRight: Reads a word from the DCache/Bus.
 * ========================================================================= */
static const uint32_t LoadWordRightMaskTable[4] = {
  0xFFFFFF00,
  0xFFFF0000,
  0xFF000000,
  0x00000000
};

void
VR4300LoadWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  int64_t *dest = (int64_t*) memoryData->target;
  unsigned type = memoryData->address & 0x3;
  size_t size = type + 1;
  int32_t word;

  word = BusReadWord(bus, memoryData->address & 0xFFFFFFFC) >> (4 - size) * 8;
  word |= (uint32_t) (*dest & LoadWordRightMaskTable[type]);

  *dest = (type != 3)
    ? (uint64_t) word
    : (int32_t) word;
}

/* ============================================================================
 *  VR4300StoreByte: Writes a byte to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreByte(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  BusWriteByte(bus, memoryData->address, memoryData->data);
}

/* ============================================================================
 *  VR4300StoreDWord: Writes a doubleword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreDWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  BusWriteDWord(bus, memoryData->address, memoryData->data);
}

/* ============================================================================
 *  VR4300StoreHWord: Writes a halfword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreHWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  BusWriteHWord(bus, memoryData->address, memoryData->data);
}

/* ============================================================================
 *  VR4300StoreWord: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  BusWriteWord(bus, memoryData->address, memoryData->data);
}

/* ============================================================================
 *  VR4300StoreWordLeft: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWordLeft(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  size_t size = 4 - (memoryData->address & 0x3);
  BusWriteWordUnaligned(bus, memoryData->address, memoryData->data, size);
}

/* ============================================================================
 *  VR4300StoreWordRight: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus) {
  size_t size = (memoryData->address & 0x3) + 1;
  BusWriteWordUnaligned(bus, memoryData->address & 0xFFFFFFFC,
    memoryData->data, size);
}

