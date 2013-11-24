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
#include "DCache.h"
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
  struct VR4300DCache *dcache = &vr4300->dcache;

  struct VR4300DCacheLine *line = NULL;
  const struct RegionInfo *region;

  VR4300MemoryFunction function = memoryData->function;

  /* Always copy the destination register. */
  dcwbLatch->result.dest = exdcLatch->result.dest;

  if (likely(function == NULL)) {
    dcwbLatch->result.data = exdcLatch->result.data;
    return;
  }

  /* Lookup the region that our address lies in. */
  memoryData->function = NULL;
  region = dcwbLatch->region;

  if ((memoryData->address - region->start) >= region->length) {
    if ((region = GetRegionInfo(vr4300, memoryData->address)) == NULL) {
      memset(&dcwbLatch->result, 0, sizeof(dcwbLatch->result));
      debug("Unimplemented fault: VR4300_FAULT_DADE.");
      return;
    }

    dcwbLatch->region = region;
  }

  memoryData->address -= region->offset;

  if (region->cached) {
    if ((line = VR4300DCacheProbe(dcache, memoryData->address)) == NULL) {
      VR4300DCacheFill(dcache, vr4300->bus, memoryData->address);
      line = VR4300DCacheProbe(dcache, memoryData->address);
    }
  }

  /* TODO: Bypass the write buffers. */
  function(memoryData, vr4300->bus, line);
}

/* ============================================================================
 *  VR4300LoadByte: Reads a byte from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadByte(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  int64_t result;
  int8_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xF), sizeof(contents));
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_BYTE, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadByteU: Reads a byte from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadByteU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  uint64_t result;
  uint8_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xF), sizeof(contents));
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_BYTE, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadDWord: Reads a doubleword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadDWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  uint64_t result;
  uint64_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0x8), sizeof(contents));
    result = ByteOrderSwap64(contents);
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_DWORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
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
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFF8;
  unsigned type = memoryData->address & 0x7;
  uint64_t mask = LoadDWordLeftMaskTable[type];
  MemoryFunction read;

  uint64_t result, olddata;
  uint64_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0x8), sizeof(contents));
    contents = ByteOrderSwap64(contents);
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_DWORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
  }

  memcpy(&olddata, memoryData->target, sizeof(olddata));
  result = (contents << (type << 3)) | (olddata & mask);
  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadDWordRight: Reads a doubleword from the DCache/Bus.
 * ========================================================================= */
static const uint64_t LoadDWordRightMaskTable[8] = {
  0xFFFFFFFFFFFFFF00ULL,
  0xFFFFFFFFFFFF0000ULL,
  0xFFFFFFFFFF000000ULL,
  0xFFFFFFFF00000000ULL,
  0xFFFFFF0000000000ULL,
  0xFFFF000000000000ULL,
  0xFF00000000000000ULL,
  0x0000000000000000ULL,
};

void
VR4300LoadDWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFF8;
  unsigned type = memoryData->address & 0x7;
  uint64_t mask = LoadDWordRightMaskTable[type];
  MemoryFunction read;

  uint64_t result, olddata;
  uint64_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0x8), sizeof(contents));
    contents = ByteOrderSwap64(contents);
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_DWORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
  }

  memcpy(&olddata, memoryData->target, sizeof(olddata));
  result = (contents >> ((7 - type) << 3)) | (olddata & mask);
  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadHWord: Reads a halfword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadHWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  int64_t result;
  int16_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xE), sizeof(contents));
    contents = ByteOrderSwap16(contents);
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_HWORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadHWordU: Reads a halfword from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadHWordU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  uint64_t result;
  uint16_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xE), sizeof(contents));
    contents = ByteOrderSwap16(contents);
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_HWORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}


/* ============================================================================
 *  VR4300LoadWord: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  int64_t result;
  int32_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xC), sizeof(contents));
    contents = ByteOrderSwap32(contents);
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadWordFPU: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadWordFPU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  uint32_t result;
  uint32_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xC), sizeof(contents));
    contents = ByteOrderSwap32(contents);
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300LoadWordU: Reads a word from the DCache/Bus.
 * ========================================================================= */
void
VR4300LoadWordU(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  MemoryFunction read;

  uint64_t result;
  uint32_t contents;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xC), sizeof(contents));
    contents = ByteOrderSwap32(contents);
    result = contents;
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
    result = contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
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
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFFC;
  unsigned type = memoryData->address & 0x3;
  uint32_t mask = LoadWordLeftMaskTable[type];
  MemoryFunction read;

  int64_t result, regolddata;
  int32_t contents, olddata;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xC), sizeof(contents));
    contents = ByteOrderSwap32(contents);
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
  }

  memcpy(&regolddata, memoryData->target, sizeof(regolddata));

  olddata = regolddata;
  contents = (contents << (type << 3)) | (olddata & mask);
  result = contents;

  memcpy(memoryData->target, &result, sizeof(result));
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
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFFC;
  unsigned type = memoryData->address & 0x3;
  uint64_t mask = LoadWordRightMaskTable[type];
  MemoryFunction read;

  uint64_t result, regolddata;
  uint32_t contents, olddata;
  void *opaque;

  if (line != NULL) {
    memcpy(&contents, line->data + (address & 0xC), sizeof(contents));
    contents = ByteOrderSwap32(contents);
  }

  else {
    if ((read = BusRead(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    read(opaque, address, &contents);
  }

  memcpy(&regolddata, memoryData->target, sizeof(regolddata));

  olddata = regolddata;
  contents = (contents >> ((3 - type) << 3)) | (olddata & mask);

  /* Sign extend the 32-bit result when type == 3. */
  /* When type != 3, do not touch the upper 32-bits. */
  if (type == 3) {
    result = (int64_t) ((int32_t) contents);
  }

  else {
    result = (regolddata & 0xFFFFFFFF00000000ULL) | contents;
  }

  memcpy(memoryData->target, &result, sizeof(result));
}

/* ============================================================================
 *  VR4300StoreByte: Writes a byte to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreByte(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint8_t contents = memoryData->data;
  MemoryFunction write;
  void *opaque;

  if (line != NULL)
    memcpy(line->data + (address & 0xF), &contents, sizeof(contents));

  else {
    if ((write = BusWrite(bus, BUS_TYPE_BYTE, address, &opaque)) == NULL)
      return;

    write(opaque, address, &contents);
  }
}

/* ============================================================================
 *  VR4300StoreDWord: Writes a doubleword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreDWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint64_t contents = memoryData->data;
  MemoryFunction write;
  void *opaque;

  if (line != NULL) {
    contents = ByteOrderSwap64(contents);
    memcpy(line->data + (address & 0x8), &contents, sizeof(contents));
  }

  else {
    if ((write = BusWrite(bus, BUS_TYPE_DWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &contents);
  }
}

/* ============================================================================
 *  VR4300StoreDWordLeft: Writes a doubleword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreDWordLeft(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint64_t contents = memoryData->data;
  struct UnalignedData data;
  MemoryFunction write;
  void *opaque;

  /* Pack the unaligned data structure. */
  /* TODO/FIXME: Take endianness into account. */
  contents = ByteOrderSwap64(contents);
  data.size = 8 - (memoryData->address & 0x7);
  memcpy(data.data, &contents, sizeof(contents));

  if (line != NULL)
    memcpy(line->data + (address & 0xF), data.data, data.size);

  else {
    if ((write = BusWrite(bus, BUS_TYPE_UDWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &data);
  }
}

/* ============================================================================
 *  VR4300DStoreWordRight: Writes a doubleword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreDWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFF8;
  uint64_t contents = memoryData->data;
  struct UnalignedData data;
  MemoryFunction write;
  void *opaque;

  /* Pack the unaligned data structure. */
  /* TODO/FIXME: Take endianness into account. */
  data.size = (memoryData->address & 0x7) + 1;
  contents = contents << ((8 - data.size) << 3);
  contents = ByteOrderSwap64(contents);
  memcpy(data.data, &contents, sizeof(contents));

  if (line != NULL)
    memcpy(line->data + (address & 0xF), data.data, data.size);

  else {
    if ((write = BusWrite(bus, BUS_TYPE_UDWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &data);
  }
}

/* ============================================================================
 *  VR4300StoreHWord: Writes a halfword to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreHWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint16_t contents = memoryData->data;
  MemoryFunction write;
  void *opaque;

  if (line != NULL) {
    contents = ByteOrderSwap16(contents);
    memcpy(line->data + (address & 0xE), &contents, sizeof(contents));
  }

  else {
    if ((write = BusWrite(bus, BUS_TYPE_HWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &contents);
  }
}

/* ============================================================================
 *  VR4300StoreWord: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWord(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint32_t contents = memoryData->data;
  MemoryFunction write;
  void *opaque;

  if (line != NULL) {
    contents = ByteOrderSwap32(contents);
    memcpy(line->data + (address & 0xC), &contents, sizeof(contents));
  }

  else {
    if ((write = BusWrite(bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &contents);
  }
}

/* ============================================================================
 *  VR4300StoreWordLeft: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWordLeft(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address;
  uint32_t contents = memoryData->data;
  struct UnalignedData data;
  MemoryFunction write;
  void *opaque;

  /* Pack the unaligned data structure. */
  /* TODO/FIXME: Take endianness into account. */
  contents = ByteOrderSwap32(contents);
  data.size = 4 - (memoryData->address & 0x3);
  memcpy(data.data, &contents, sizeof(contents));

  if (line != NULL)
    memcpy(line->data + (address & 0xF), data.data, data.size);

  else {
    if ((write = BusWrite(bus, BUS_TYPE_UWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &data);
  }
}

/* ============================================================================
 *  VR4300StoreWordRight: Writes a word to the DCache/Bus.
 * ========================================================================= */
void
VR4300StoreWordRight(const struct VR4300MemoryData *memoryData,
  struct BusController *bus, struct VR4300DCacheLine *line) {
  uint32_t address = memoryData->address & 0xFFFFFFFC;
  uint32_t contents = memoryData->data;
  struct UnalignedData data;
  MemoryFunction write;
  void *opaque;

  /* Pack the unaligned data structure. */
  /* TODO/FIXME: Take endianness into account. */
  data.size = (memoryData->address & 0x3) + 1;
  contents = contents << ((4 - data.size) << 3);
  contents = ByteOrderSwap32(contents);
  memcpy(data.data, &contents, sizeof(contents));

  if (line != NULL)
    memcpy(line->data + (address & 0xF), data.data, data.size);

  else {
    if ((write = BusWrite(bus, BUS_TYPE_UWORD, address, &opaque)) == NULL)
      return;

    write(opaque, address, &data);
  }
}

