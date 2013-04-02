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

#define NUM_FAULT_QUEUE_NODES 5

enum VR4300PipelineFault {
#define X(fault) VR4300_FAULT_##fault,
#include "Fault.md"
  NUM_VR4300_FAULTS
#undef X
};

struct VR4300;
struct VR4300Pipeline;

typedef void (*const FaultHandler)(struct VR4300 *);

struct VR4300FaultQueueNode {
  struct VR4300FaultQueueNode *next;
  enum VR4300PipelineFault faultType;
};

struct VR4300FaultQueue {
  struct VR4300FaultQueueNode nodes[NUM_FAULT_QUEUE_NODES];
  struct VR4300FaultQueueNode *head, *freeHead;
};

#ifndef NDEBUG
extern const char *VR4300FaultMnemonics[NUM_VR4300_FAULTS];
#endif

extern const FaultHandler FaultHandlerTable[NUM_VR4300_FAULTS];


void HandleFaults(struct VR4300 *);
void InitFaultQueue(struct VR4300FaultQueue *);
void QueueFault(struct VR4300FaultQueue *, enum VR4300PipelineFault);

#endif

