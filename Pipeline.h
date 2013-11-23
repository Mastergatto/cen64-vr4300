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
#include "Latches.h"
#include "Region.h"

struct VR4300Pipeline {
  unsigned stalls, padding;

  struct VR4300ICRFLatch icrfLatch;
  struct VR4300RFEXLatch rfexLatch;
  struct VR4300EXDCLatch exdcLatch;
  struct VR4300DCWBLatch dcwbLatch;
  struct VR4300FaultManager faultManager;

  unsigned long long cycles;
};

struct VR4300;

void CycleVR4300(struct VR4300 *);
void VR4300InitPipeline(struct VR4300Pipeline *);

#endif

