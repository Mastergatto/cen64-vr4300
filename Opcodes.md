/* ============================================================================
 *  Opcodes.md: Opcode types, info, and other data.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef VR4300_OPCODE_TABLE
#define VR4300_OPCODE_TABLE X(INV) \
  X(ADD) X(ADDI) X(ADDIU) X(ADDU) X(AND) X(ANDI) X(BC0) X(BC1) X(BC2) \
  X(BEQ) X(BEQL) X(BGEZ) X(BGEZAL) X(BGEZALL) X(BGEZL) X(BGTZ) X(BGTZL) \
  X(BLEZ) X(BLEZL) X(BLTZ) X(BLTZAL) X(BLTZALL) X(BLTZL) X(BNE) X(BNEL) \
  X(BREAK) X(CACHE) X(CFC0) X(CFC1) X(CFC2) X(COP0) X(COP1) X(COP2) \
  X(CTC0) X(CTC1) X(CTC2) X(DADD) X(DADDI) X(DADDIU) X(DADDU) X(DDIV) \
  X(DDIVU) X(DIV) X(DIVU) X(DMFC0) X(DMFC1) X(DMFC2) X(DMTC0) X(DMTC1) \
  X(DMTC2) X(DMULT) X(DMULTU) X(DSLL) X(DSLLV) X(DSLL32) X(DSRA) X(DSRAV) \
  X(DSRA32) X(DSRL) X(DSRLV) X(DSRL32) X(DSUB) X(DSUBU) X(FPUD) X(FPUL) \
  X(FPUS) X(FPUW) X(J) X(JAL) X(JALR) X(JR) X(LB) X(LBU) X(LD) X(LDC0) \
  X(LDC1) X(LDC2) X(LDL) X(LDR) X(LH) X(LHU) X(LL) X(LLD) X(LUI) X(LW) \
  X(LWC0) X(LWC1) X(LWC2) X(LWL) X(LWR) X(LWU) X(MFC0) X(MFC1) X(MFC2) \
  X(MFHI) X(MFLO) X(MTC0) X(MTC1) X(MTC2) X(MTHI) X(MTLO) X(MULT) \
  X(MULTU) X(NOR) X(OR) X(ORI) X(SB) X(SC) X(SCD) X(SD) X(SDC0) X(SDC1) \
  X(SDC2) X(SDL) X(SDR) X(SH) X(SLL) X(SLLV) X(SLT) X(SLTI) X(SLTIU) \
  X(SLTU) X(SRA) X(SRAV) X(SRL) X(SRLV) X(SUB) X(SUBU) X(SW) X(SWC0) \
  X(SWC1) X(SWC2) X(SWL) X(SWR) X(SYNC) X(SYSCALL) X(TEQ) X(TEQI) X(TGE) \
  X(TGEI) X(TGEIU) X(TGEU) X(TLB) X(TLT) X(TLTI) X(TLTIU) X(TLTU) X(TNE) \
  X(TNEI) X(XOR) X(XORI)
#endif

VR4300_OPCODE_TABLE

