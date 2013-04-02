/* ============================================================================
 *  Externs.h: External definitions for the VR4300 plugin.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__EXTERNS_H__
#define __VR4300__EXTERNS_H__
#include <stddef.h>

struct BusController;

uint8_t BusReadByte(const struct BusController *, uint32_t);
uint64_t BusReadDWord(const struct BusController *, uint32_t);
uint16_t BusReadHWord(const struct BusController *, uint32_t);
uint32_t BusReadWord(const struct BusController *, uint32_t);
void BusWriteByte(const struct BusController *, uint32_t, uint8_t);
void BusWriteDWord(const struct BusController *, uint32_t, uint64_t);
void BusWriteHWord(const struct BusController *, uint32_t, uint16_t);
void BusWriteWord(const struct BusController *, uint32_t, uint32_t);

uint32_t BusReadWordUnaligned(const struct BusController *, uint32_t, size_t);
void BusWriteWordUnaligned(const struct BusController *,
  uint32_t, uint32_t, size_t);

#endif

