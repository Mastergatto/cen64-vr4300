/* ============================================================================
 *  Decoder.c: Instruction decoder.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "Decoder.h"
#include "Opcodes.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

/* These will only touch processor cachelines */
/* if an invalid/undefined instruction is used. */
static const struct VR4300Opcode InvalidOpcodeTable[64] = {
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: Special.
 *
 *      31---------26------------------------------------------5--------0
 *      | SPECIAL/6 |                                         |  FMT/6  |
 *      ------6----------------------------------------------------6-----
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 | SLL   |       | SRL   | SRA   | SLLV  |       | SRLV  | SRAV  |
 *  001 | JR    | JALR  |       |       |SYSCALL| BREAK |       | SYNC  |
 *  010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |       | DSRLV | DSRAV |
 *  011 | MULT  | MULTU | DIV   | DIVU  | DMULT | DMULTU| DDIV  | DDIVU |
 *  100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   |
 *  101 |       |       | SLT   | SLTU  | DADD  | DADDU | DSUB  | DSUBU |
 *  110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |       | TNE   |       |
 *  111 | DSLL  |       | DSRL  | DSRA  |DSLL32 |       |DSRL32 |DSRA32 |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct VR4300Opcode SpecialOpcodeTable[64] = {
  {SLL},     {INVALID}, {SRL},     {SRA},
  {SLLV},    {INVALID}, {SRLV},    {SRAV},
  {JR},      {JALR},    {INVALID}, {INVALID},
  {SYSCALL}, {BREAK},   {INVALID}, {SYNC},
  {MFHI},    {MTHI},    {MFLO},    {MTLO},
  {DSLLV},   {INVALID}, {DSRLV},   {DSRAV},
  {MULT},    {MULTU},   {DIV},     {DIVU},
  {DMULT},   {DMULTU},  {DDIV},    {DDIVU},
  {ADD},     {ADDU},    {SUB},     {SUBU},
  {AND},     {OR},      {XOR},     {NOR},
  {INVALID}, {INVALID}, {SLT},     {SLTU},
  {DADD},    {DADDU},   {DSUB},    {DSUBU},
  {TGE},     {TGEU},    {TLT},     {TLTU},
  {TEQ},     {INVALID}, {TNE},     {INVALID},
  {DSLL},    {INVALID}, {DSRL},    {DSRA},
  {DSLL32},  {INVALID}, {DSRL32},  {DSRA32}
};

/* ============================================================================
 *  Escaped opcode table: RegImm.
 *
 *      31---------26----------20-------16------------------------------0
 *      | = REGIMM  |          |  FMT/5  |                              |
 *      ------6---------------------5------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | BLTZ  | BGEZ  | BLTZL | BGEZL |       |       |       |       |
 *   01 | TGEI  | TGEIU | TLTI  | TLTIU | TEQI  |       | TNEI  |       |
 *   10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|       |       |       |       |
 *   11 |       |       |       |       |       |       |       |       |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct VR4300Opcode RegImmOpcodeTable[32] = {
  {BLTZ},    {BGEZ},    {BLTZL},   {BGEZL},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {TGEI},    {TGEIU},   {TLTI},    {TLTIU},
  {TEQI},    {INVALID}, {TNEI},    {INVALID},
  {BLTZAL},  {BGEZAL},  {BLTZALL}, {BGEZALL},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: COP0.
 *
 *      31--------26-25------21 ----------------------------------------0
 *      |  COP0/6   |  FMT/5  |                                         |
 *      ------6----------5-----------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | MFC0  | DMFC0 | CFC0  |  ---  | MTC0  | DMTC0 | CTC0  |  ---  |
 *   01 |  BC0  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 |  TLB  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 * ========================================================================= */
static const struct VR4300Opcode COP0OpcodeTable[32] = {
  {MFC0},    {DMFC0},   {CFC0},    {INVALID},
  {MTC0},    {DMTC0},   {CTC0},    {INVALID},
  {BC0},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {TLB},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: COP1.
 *
 *      31--------26-25------21 ----------------------------------------0
 *      |  COP1/6   |  FMT/5  |                                         |
 *      ------6----------5-----------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | MFC1  | DMFC1 | CFC1  |  ---  | MTC1  | DMTC1 | CTC1  |  ---  |
 *   01 |  BC1  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 | FPUS  | FPUD  |  ---  |  ---  | FPUW  | FPUL  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 * ========================================================================= */
static const struct VR4300Opcode COP1OpcodeTable[32] = {
  {MFC1},    {DMFC1},   {CFC1},    {INVALID},
  {MTC1},    {DMTC1},   {CTC1},    {INVALID},
  {BC1},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {FPUS},    {FPUD},    {INVALID}, {INVALID},
  {FPUW},    {FPUL},    {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: COP2.
 *  TODO: Does this even exist?
 *
 *      31--------26-25------21 ----------------------------------------0
 *      |  COP2/6   |  FMT/5  |                                         |
 *      ------6----------5-----------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | MFC2  | DMFC2 | CFC2  |  ---  | MTC2  | DMTC2 | CTC2  |  ---  |
 *   01 |  BC2  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct VR4300Opcode COP2OpcodeTable[32] = {
  {MFC2},    {DMFC2},   {CFC2},    {INVALID},
  {MTC2},    {DMTC2},   {CTC2},    {INVALID},
  {BC2},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  First-order opcode table.
 *
 *  An OPCODE of 0b000000   => Lookup in SpecialOpcodeTable.
 *  An OPCODE of 0b000001   => Lookup in RegImmOpcodeTable.
 *  An OPCODE of 0b010000   => Lookup in COP0OpcodeTable.
 *  An OPCODE of 0b010001   => Lookup in COP0OpcodeTable.
 *  An OPCODE of 0b010010   => Lookup in COP2OpcodeTable.
 *
 *      31---------26---------------------------------------------------0
 *      |  OPCODE/6 |                                                   |
 *      ------6----------------------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 | *SPEC | *RGIM | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  |
 *  001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   |
 *  010 | *COP0 | *COP1 | *COP2 |       | BEQL  | BNEL  | BLEZL | BGTZL |
 *  011 | DADDI |DADDIU |  LDL  |  LDR  |       |       |       |       |
 *  100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | LWU   |
 *  101 | SB    | SH    | SWL   | SW    | SDL   | SDR   | SWR   | CACHE |
 *  110 | LL    | LWC1  | LWC2  |       | LLD   | LDC1  | LDC2  | LD    |
 *  111 | SC    | SWC1  | SWC2  |       | SCD   | SDC1  | SDC2  | SD    |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct VR4300Opcode OpcodeTable[64] = {
  {INVALID}, {INVALID}, {J},       {JAL},
  {BEQ},     {BNE},     {BLEZ},    {BGTZ},
  {ADDI},    {ADDIU},   {SLTI},    {SLTIU},
  {ANDI},    {ORI},     {XORI},    {LUI},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {BEQL},    {BNEL},    {BLEZL},   {BGTZL},
  {DADDI},   {DADDIU},  {LDL},     {LDR},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {LB},      {LH},      {LWL},     {LW},
  {LBU},     {LHU},     {LWR},     {LWU},
  {SB},      {SH},      {SWL},     {SW},
  {SDL},     {SDR},     {SWR},     {CACHE},
  {LL},      {LWC1},    {LWC2},    {INVALID},
  {LLD},     {LDC1},    {LDC2},    {LD},
  {SC},      {SWC1},    {SWC2},    {INVALID},
  {SCD},     {SDC1},    {SDC2},    {SD}
};

/* Escaped table listings. Most of these will never */
/* see a processor cacheline, so not much waste here. */
static const struct VR4300OpcodeEscape EscapeTable[64] = {
  {SpecialOpcodeTable,  0, 0x3F}, {RegImmOpcodeTable,  16, 0x1F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {COP0OpcodeTable,    21, 0x1F}, {COP1OpcodeTable,    21, 0x1F},
  {COP2OpcodeTable,    21, 0x1F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},

  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
  {OpcodeTable,        26, 0x3F}, {OpcodeTable,        26, 0x3F},
};

/* ============================================================================
 *  VR4300DecodeInstruction: Looks up an instruction in the opcode table.
 *  Instruction words are assumed to be in big-endian byte order.
 * ========================================================================= */
const struct VR4300Opcode*
VR4300DecodeInstruction(uint32_t iw) {
  const struct VR4300OpcodeEscape *escape = &EscapeTable[iw >> 26];
  unsigned index = iw >> escape->shift & escape->mask;

  return &escape->table[index];
}

/* ============================================================================
 *  VR4300InvalidateOpcode: Invalidates an opcode.
 * ========================================================================= */
void
VR4300InvalidateOpcode(struct VR4300Opcode *opcode) {
  memset(opcode, 0, sizeof(*opcode));
}

