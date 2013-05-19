/* ============================================================================
 *  Region.h: Memory address space resolver.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__REGION_H__
#define __VR4300__REGION_H__
#include "Common.h"

struct VR4300;
struct RegionInfo {
  uint64_t offset;
  uint64_t start;
  uint64_t length;
  bool cached;
};

const struct RegionInfo* GetDefaultRegion(void);
const struct RegionInfo* GetRegionInfo(const struct VR4300 *, uint64_t);

#endif

