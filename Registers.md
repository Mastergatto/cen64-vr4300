/* ============================================================================
 *  Registers.md: MIPS interface registers.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef MI_REGISTER_LIST
#define MI_REGISTER_LIST \
  X(MI_INIT_MODE_REG) \
  X(MI_VERSION_REG) \
  X(MI_INTR_REG) \
  X(MI_INTR_MASK_REG)
#endif

MI_REGISTER_LIST

