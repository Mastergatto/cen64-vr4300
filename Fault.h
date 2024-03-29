/* ============================================================================
 *  Fault.h: Fault and exception handler.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__FAULT_H__
#define __VR4300__FAULT_H__
#include "Common.h"
#include "Latches.h"

enum VR4300PipelineFault {
#define X(fault) VR4300_FAULT_##fault,
#include "Fault.md"
  NUM_VR4300_FAULTS
#undef X
};

struct VR4300;
struct VR4300Pipeline;

enum VR4300PCUIndex {
  VR4300_PCU_NORMAL = -1,
  VR4300_PCU_START_RF = 0,
  VR4300_PCU_START_EX = 1,
  VR4300_PCU_START_DC = 2,
  VR4300_PCU_RESUME_IC = 4,
  VR4300_PCU_RESUME_RF = 5,
  VR4300_PCU_RESUME_EX = 6,
  VR4300_PCU_RESUME_DC = 7,
#ifdef DO_FASTFORWARD
  VR4300_PCU_FASTFORWARD = 8,
#endif
};

struct VR4300FaultManager {
  struct VR4300ICRFLatch savedIcrfLatch;

  uint64_t faultingPC;
  uint32_t nextOpcodeFlags;
  uint32_t excpCauseData;
  uint32_t ilData;

  enum VR4300PipelineFault excp;
  enum VR4300PipelineFault il;
  enum VR4300PCUIndex excpIndex;
  enum VR4300PCUIndex ilIndex;
  bool faulting;
};

#ifndef NDEBUG
extern const char *VR4300FaultMnemonics[NUM_VR4300_FAULTS];
#endif

void InitFaultManager(struct VR4300FaultManager *manager);
void HandleExceptions(struct VR4300 *vr4300);
void HandleInterlocks(struct VR4300 *vr4300);

void QueueException(struct VR4300FaultManager *manager,
  enum VR4300PipelineFault fault, uint64_t faultingPC,
  uint32_t nextOpcodeFlags, uint32_t faultCauseData,
  enum VR4300PCUIndex pcuIndex);

void QueueInterlock(struct VR4300Pipeline *pipeline,
  enum VR4300PipelineFault fault, uint32_t faultCauseData,
  enum VR4300PCUIndex pcuIndex);

void PerformHardReset(struct VR4300FaultManager *manager);
void PerformSoftReset(struct VR4300FaultManager *manager);

#endif

