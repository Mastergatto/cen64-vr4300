/* ============================================================================
 *  WBStage.h: Writeback stage.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__WBSTAGE_H__
#define __VR4300__WBSTAGE_H__
#include "Common.h"
#include "Pipeline.h"

void VR4300WBStage(const struct VR4300DCWBLatch *, uint64_t[]);

#endif

