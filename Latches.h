/* ============================================================================
 *  Pipeline.h: Processor pipeline latch definitions.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __LATCHES_H__
#define __LATCHES_H__
#include "Common.h"

struct VR4300Result {
  uint64_t data;
  uint32_t dest;
  uint32_t flags;
};

struct VR4300ICRFLatch {
  const struct RegionInfo *region;

  uint64_t pc;
  uint32_t address;
  uint32_t iwMask;
};

struct VR4300RFEXLatch {
  uint64_t pc;
  struct VR4300Opcode opcode;
  uint32_t iw, padding;
};

struct VR4300EXDCLatch {
  struct VR4300Result result;
  struct VR4300MemoryData memoryData;
};

struct VR4300DCWBLatch {
  struct VR4300Result result;
  const struct RegionInfo *region;
};

#endif


