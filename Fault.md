/* ============================================================================
 *  Fault.md: Fault and exception handler.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013 Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */

#ifndef VR4300_FAULT_LIST
#define VR4300_FAULT_LIST \
  X(CP0I) X(RST) X(NMI) X(OVFL) X(TRAP) X(FPE) X(DADE) X(DTLB) X(WAT) \
  X(INTR) X(DCM) X(DCB) X(COP) X(DBE) X(SYSC) X(BRPT) X(CPU) X(RSVD) \
  X(LDI) X(MCI) X(IADE) X(ITM) X(ICB) X(IBE)
#endif

VR4300_FAULT_LIST

