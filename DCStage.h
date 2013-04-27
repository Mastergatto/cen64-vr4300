/* ============================================================================
 *  DCStage.h: Data cache fetch stage.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__DCSTAGE_H__
#define __VR4300__DCSTAGE_H__
#include "Common.h"

struct BusController;
struct VR4300MemoryData;

typedef void (*VR4300MemoryFunction)(const struct VR4300MemoryData *,
  struct BusController *);

struct VR4300MemoryData {
  VR4300MemoryFunction function;

  uint64_t address;
  uint64_t data;
  void *target;
};

struct VR4300EXDFLatch;
struct VR4300DFWBLatch;

void VR4300DCStage(struct VR4300 *);

/* Memory functions. */
void VR4300LoadByte(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadByteU(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadHWord(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadHWordU(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadWord(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadDWord(const struct VR4300MemoryData *, struct BusController *);
void VR4300StoreByte(const struct VR4300MemoryData *, struct BusController *);
void VR4300StoreHWord(const struct VR4300MemoryData *, struct BusController *);
void VR4300StoreWord(const struct VR4300MemoryData *, struct BusController *);
void VR4300StoreDWord(const struct VR4300MemoryData *, struct BusController *);

void VR4300LoadWordFPU(const struct VR4300MemoryData *, struct BusController *);
void VR4300LoadDWordFPU(const struct VR4300MemoryData *, struct BusController *);

/* Unaligned accesses. */
void VR4300LoadWordLeft(const struct VR4300MemoryData *,
  struct BusController *);
void VR4300LoadWordRight(const struct VR4300MemoryData *,
  struct BusController *);
void VR4300StoreWordLeft(const struct VR4300MemoryData *,
  struct BusController *);
void VR4300StoreWordRight(const struct VR4300MemoryData *,
  struct BusController *);
void VR4300LoadDWordLeft(const struct VR4300MemoryData *,
  struct BusController *);
void VR4300LoadDWordRight(const struct VR4300MemoryData *,
  struct BusController *);

#endif

