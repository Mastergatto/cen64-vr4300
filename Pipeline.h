/* ============================================================================
 *  Pipeline.h: Processor pipeline.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__PIPELINE_H__
#define __VR4300__PIPELINE_H__
#include "Common.h"
#include "Decoder.h"
#include "DCStage.h"
#include "Fault.h"
#include "Region.h"

enum VR4300PipelineStage {
  VR4300_PIPELINE_STAGE_IC,
  VR4300_PIPELINE_STAGE_RF,
  VR4300_PIPELINE_STAGE_EX,
  VR4300_PIPELINE_STAGE_DC,
  VR4300_PIPELINE_STAGE_WB,
  NUM_VR4300_PIPELINE_STAGES
};

struct VR4300PendingException {
  uint64_t faultingPC;
  uint32_t nextOpcodeFlags;
  uint32_t causeData;
};

struct VR4300Result {
  uint64_t data, dest;
};

struct VR4300ICRFLatch {
  const struct RegionInfo *region;

  uint64_t pc;
  uint32_t address;
  uint32_t iwMask;
};

struct VR4300RFEXLatch {
  struct VR4300Opcode opcode;
  uint32_t iw;
};

struct VR4300EXDCLatch {
  struct VR4300Result result;
  struct VR4300MemoryData memoryData;
};

struct VR4300DCWBLatch {
  const struct RegionInfo *region;
  struct VR4300Result result;
};

struct VR4300Pipeline {
  unsigned long long cycles;
  unsigned long long stalls;

  struct VR4300ICRFLatch icrfLatch;
  struct VR4300RFEXLatch rfexLatch;
  struct VR4300EXDCLatch exdcLatch;
  struct VR4300DCWBLatch dcwbLatch;
  struct VR4300FaultQueue faultQueue;

  struct VR4300PendingException exception;
  enum VR4300PipelineStage startStage;
};

struct VR4300;

void CycleVR4300(struct VR4300 *);
void VR4300InitPipeline(struct VR4300Pipeline *);

#endif

