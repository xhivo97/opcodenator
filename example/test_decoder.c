#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "decoder.h"

typedef enum {
  TEST_ADC,
  TEST_ADD,
  TEST_ADIW,
  TEST_AND,
  TEST_ANDI,
  TEST_ASR,
  TEST_BCLR,
  TEST_BLD,
  TEST_BRBC,
  TEST_BRBS,
  TEST_BRCC,
  TEST_BRCS,
  TEST_BREAK,
  TEST_BREQ,
  TEST_BRGE,
  TEST_BRHC,
  TEST_BRHS,
  TEST_BRID,
  TEST_BRIE,
  TEST_BRLO,
  TEST_BRLT,
  TEST_BRMI,
  TEST_BRNE,
  TEST_BRPL,
  TEST_BRSH,
  TEST_BRTC,
  TEST_BRTS,
  TEST_BRVC,
  TEST_BRVS,
  TEST_BSET,
  TEST_BST,
  TEST_CALL,
  TEST_CBI,
  TEST_CBR,
  TEST_CLC,
  TEST_CLH,
  TEST_CLI,
  TEST_CLN,
  TEST_CLR,
  TEST_CLS,
  TEST_CLT,
  TEST_CLV,
  TEST_CLZ,
  TEST_COM,
  TEST_CP,
  TEST_CPC,
  TEST_CPI,
  TEST_CPSE,
  TEST_DEC,
  TEST_DES,
  TEST_EICALL,
  TEST_EIJMP,
  TEST_ELPM,
  TEST_ELPM_R0,
  TEST_ELPM_INC,
  TEST_EOR,
  TEST_FMUL,
  TEST_FMULS,
  TEST_FMULSU,
  TEST_ICALL,
  TEST_IJMP,
  TEST_IN,
  TEST_INC,
  TEST_JMP,
  TEST_LAC,
  TEST_LAS,
  TEST_LAT,
  TEST_LD_X,
  TEST_LD_X_INC,
  TEST_LD_X_DEC,
  TEST_LD_Y,
  TEST_LD_Y_INC,
  TEST_LD_Y_DEC,
  TEST_LDD_Y,
  TEST_LD_Z,
  TEST_LD_Z_INC,
  TEST_LD_Z_DEC,
  TEST_LDD_Z,
  TEST_LDI,
  TEST_LDS32,
  TEST_LDS16,
  TEST_LPM,
  TEST_LPM_Z,
  TEST_LPM_Z_INC,
  TEST_LSL,
  TEST_LSR,
  TEST_MOV,
  TEST_MOVW,
  TEST_MUL,
  TEST_MULS,
  TEST_MULSU,
  TEST_NEG,
  TEST_NOP,
  TEST_OR,
  TEST_ORI,
  TEST_OUT,
  TEST_POP,
  TEST_PUSH,
  TEST_RCALL,
  TEST_RET,
  TEST_RETI,
  TEST_RJMP,
  TEST_ROL,
  TEST_ROR,
  TEST_SBC,
  TEST_SBCI,
  TEST_SBI,
  TEST_SBIC,
  TEST_SBIS,
  TEST_SBIW,
  TEST_SBR,
  TEST_SBRC,
  TEST_SBRS,
  TEST_SEC,
  TEST_SEH,
  TEST_SEI,
  TEST_SEN,
  TEST_SER,
  TEST_SES,
  TEST_SET,
  TEST_SEV,
  TEST_SEZ,
  TEST_SLEEP,
  TEST_SPM1_INC,
  TEST_SPM2,
  TEST_SPM2_INC,
  TEST_ST_X,
  TEST_ST_X_INC,
  TEST_ST_X_DEC,
  TEST_ST_Y,
  TEST_ST_Y_INC,
  TEST_ST_Y_DEC,
  TEST_STD_Y,
  TEST_ST_Z,
  TEST_ST_Z_INC,
  TEST_ST_Z_DEC,
  TEST_STD_Z,
  TEST_STS32,
  TEST_STS16,
  TEST_SUB,
  TEST_SUBI,
  TEST_SWAP,
  TEST_TST,
  TEST_WDR,
  TEST_XCH,
  NUM_TEST_OPCODES,
} TestingOpcodeType;

typedef struct {
  char name[32];
  uint32_t value;
  uint32_t mask;
  OpcodeType expected;
} TestingOpcodeData;

static const TestingOpcodeData test_opcodes[] = {
  [TEST_ADC]       = { .name = "ADC",       .value = 0x1C000000, .mask = 0x03FF0000, .expected = ADC          },
  [TEST_ADD]       = { .name = "ADD",       .value = 0x0C000000, .mask = 0x03FF0000, .expected = ADD          },
  [TEST_ADIW]      = { .name = "ADIW",      .value = 0x96000000, .mask = 0x00FF0000, .expected = ADIW         },
  [TEST_AND]       = { .name = "AND",       .value = 0x20000000, .mask = 0x03FF0000, .expected = AND          },
  [TEST_ANDI]      = { .name = "ANDI",      .value = 0x70000000, .mask = 0x0FFF0000, .expected = ANDI         },
  [TEST_ASR]       = { .name = "ASR",       .value = 0x94050000, .mask = 0x01F00000, .expected = ASR          },
  [TEST_BCLR]      = { .name = "BCLR",      .value = 0x94880000, .mask = 0x00700000, .expected = BCLR         },
  [TEST_BLD]       = { .name = "BLD",       .value = 0xF8000000, .mask = 0x01F70000, .expected = BLD          },
  [TEST_BRBC]      = { .name = "BRBC",      .value = 0xF4000000, .mask = 0x03FF0000, .expected = BRBC         },
  [TEST_BRBS]      = { .name = "BRBS",      .value = 0xF0000000, .mask = 0x03FF0000, .expected = BRBS         },
  [TEST_BRCC]      = { .name = "BRCC",      .value = 0xF4000000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRCS]      = { .name = "BRCS",      .value = 0xF0000000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BREAK]     = { .name = "BREAK",     .value = 0x95980000, .mask = 0x00000000, .expected = BREAK        },
  [TEST_BREQ]      = { .name = "BREQ",      .value = 0xF0010000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRGE]      = { .name = "BRGE",      .value = 0xF4040000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRHC]      = { .name = "BRHC",      .value = 0xF4050000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRHS]      = { .name = "BRHS",      .value = 0xF0050000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRID]      = { .name = "BRID",      .value = 0xF4070000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRIE]      = { .name = "BRIE",      .value = 0xF0070000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRLO]      = { .name = "BRLO",      .value = 0xF0000000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRLT]      = { .name = "BRLT",      .value = 0xF0040000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRMI]      = { .name = "BRMI",      .value = 0xF0020000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRNE]      = { .name = "BRNE",      .value = 0xF4010000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRPL]      = { .name = "BRPL",      .value = 0xF4020000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRSH]      = { .name = "BRSH",      .value = 0xF4000000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRTC]      = { .name = "BRTC",      .value = 0xF4060000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRTS]      = { .name = "BRTS",      .value = 0xF0060000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BRVC]      = { .name = "BRVC",      .value = 0xF4030000, .mask = 0x03F80000, .expected = BRBC         },
  [TEST_BRVS]      = { .name = "BRVS",      .value = 0xF0030000, .mask = 0x03F80000, .expected = BRBS         },
  [TEST_BSET]      = { .name = "BSET",      .value = 0x94080000, .mask = 0x00700000, .expected = BSET         },
  [TEST_BST]       = { .name = "BST",       .value = 0xFA000000, .mask = 0x01F70000, .expected = BST          },
  [TEST_CALL]      = { .name = "CALL",      .value = 0x940E0000, .mask = 0x01F1FFFF, .expected = CALL         },
  [TEST_CBI]       = { .name = "CBI",       .value = 0x98000000, .mask = 0x00FF0000, .expected = SBI_CBI      },
  [TEST_CBR]       = { .name = "CBR",       .value = 0x70000000, .mask = 0x0FFF0000, .expected = ANDI         },
  [TEST_CLC]       = { .name = "CLC",       .value = 0x94880000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLH]       = { .name = "CLH",       .value = 0x94D80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLI]       = { .name = "CLI",       .value = 0x94F80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLN]       = { .name = "CLN",       .value = 0x94A80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLR]       = { .name = "CLR",       .value = 0x24000000, .mask = 0x03FF0000, .expected = EOR          },
  [TEST_CLS]       = { .name = "CLS",       .value = 0x94C80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLT]       = { .name = "CLT",       .value = 0x94E80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLV]       = { .name = "CLT",       .value = 0x94D80000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_CLZ]       = { .name = "CLZ",       .value = 0x94980000, .mask = 0x00000000, .expected = BCLR         },
  [TEST_COM]       = { .name = "COM",       .value = 0x94000000, .mask = 0x01F00000, .expected = COM          },
  [TEST_CP]        = { .name = "CP",        .value = 0x14000000, .mask = 0x03FF0000, .expected = CP           },
  [TEST_CPC]       = { .name = "CPC",       .value = 0x04000000, .mask = 0x03FF0000, .expected = CPC          },
  [TEST_CPI]       = { .name = "CPI",       .value = 0x30000000, .mask = 0x0FFF0000, .expected = CPI          },
  [TEST_CPSE]      = { .name = "CPSE",      .value = 0x10000000, .mask = 0x03FF0000, .expected = CPSE         },
  [TEST_DEC]       = { .name = "DEC",       .value = 0x940A0000, .mask = 0x01F00000, .expected = DEC          },
  [TEST_DES]       = { .name = "DES",       .value = 0x940B0000, .mask = 0x00F00000, .expected = DES          },
  [TEST_EICALL]    = { .name = "EICALL",    .value = 0x95190000, .mask = 0x00000000, .expected = EICALL       },
  [TEST_EIJMP]     = { .name = "EIJMP",     .value = 0x94190000, .mask = 0x00000000, .expected = EIJMP        },
  [TEST_ELPM]      = { .name = "ELPM",      .value = 0x90060000, .mask = 0x01F00000, .expected = LPM          },
  [TEST_ELPM_R0]   = { .name = "ELPM_R0",   .value = 0x95D80000, .mask = 0x00000000, .expected = LPM_R0       },
  [TEST_ELPM_INC]  = { .name = "ELPM_INC",  .value = 0x90070000, .mask = 0x01F00000, .expected = LPM          },
  [TEST_EOR]       = { .name = "EOR",       .value = 0x24000000, .mask = 0x03FF0000, .expected = EOR          },
  [TEST_FMUL]      = { .name = "FMUL",      .value = 0x03080000, .mask = 0x00770000, .expected = FMUL         },
  [TEST_FMULS]     = { .name = "FMULS",     .value = 0x03800000, .mask = 0x00770000, .expected = FMULS        },
  [TEST_FMULSU]    = { .name = "FMULSU",    .value = 0x03880000, .mask = 0x00770000, .expected = FMULSU       },
  [TEST_ICALL]     = { .name = "ICALL",     .value = 0x95090000, .mask = 0x00000000, .expected = ICALL        },
  [TEST_IJMP]      = { .name = "IJMP",      .value = 0x94090000, .mask = 0x00000000, .expected = IJMP         },
  [TEST_IN]        = { .name = "IN",        .value = 0xB0000000, .mask = 0x07FF0000, .expected = IN           },
  [TEST_INC]       = { .name = "INC",       .value = 0x94030000, .mask = 0x01F00000, .expected = INC          },
  [TEST_JMP]       = { .name = "JMP",       .value = 0x940C0000, .mask = 0x01F1FFFF, .expected = JMP          },
  [TEST_LAC]       = { .name = "LAC",       .value = 0x92060000, .mask = 0x01F00000, .expected = LAC          },
  [TEST_LAS]       = { .name = "LAS",       .value = 0x92050000, .mask = 0x01F00000, .expected = LAS          },
  [TEST_LAT]       = { .name = "LAT",       .value = 0x92070000, .mask = 0x01F00000, .expected = LAT          },
  [TEST_LD_X]      = { .name = "LD_X",      .value = 0x900C0000, .mask = 0x01F00000, .expected = INDIR_X      },
  [TEST_LD_X_INC]  = { .name = "LD_X_INC",  .value = 0x900D0000, .mask = 0x01F00000, .expected = INDIR_X_INC  },
  [TEST_LD_X_DEC]  = { .name = "LD_X_DEC",  .value = 0x900E0000, .mask = 0x01F00000, .expected = INDIR_X_DEC  },
  [TEST_LD_Y]      = { .name = "LD_Y",      .value = 0x80080000, .mask = 0x01F00000, .expected = INDIR_DP_D16 },
  [TEST_LD_Y_INC]  = { .name = "LD_Y_INC",  .value = 0x90090000, .mask = 0x01F00000, .expected = INDIR_Y_INC  },
  [TEST_LD_Y_DEC]  = { .name = "LD_Y_DEC",  .value = 0x900A0000, .mask = 0x01F00000, .expected = INDIR_Y_DEC  },
  [TEST_LDD_Y]     = { .name = "LDD_Y",     .value = 0x80080000, .mask = 0x2DFF0000, .expected = INDIR_DP_D16 },
  [TEST_LD_Z]      = { .name = "LD_Z",      .value = 0x80000000, .mask = 0x01F00000, .expected = INDIR_DP_D16 },
  [TEST_LD_Z_INC]  = { .name = "LD_Z_INC",  .value = 0x90010000, .mask = 0x01F00000, .expected = INDIR_Z_INC  },
  [TEST_LD_Z_DEC]  = { .name = "LD_Z_DEC",  .value = 0x90020000, .mask = 0x01F00000, .expected = INDIR_Z_DEC  },
  [TEST_LDD_Z]     = { .name = "LDD_Z",     .value = 0x80000000, .mask = 0x2DFF0000, .expected = INDIR_DP_D16 },
  [TEST_LDI]       = { .name = "LDI",       .value = 0xE0000000, .mask = 0x0FFF0000, .expected = LDI          },
  [TEST_LDS32]     = { .name = "LDS32",     .value = 0x90000000, .mask = 0x01F0FFFF, .expected = LDS32        },
  [TEST_LDS16]     = { .name = "LDS16",     .value = 0xA0000000, .mask = 0x07FF0000, .expected = INDIR_DP_D16 },
  [TEST_LPM]       = { .name = "LPM",       .value = 0x95C80000, .mask = 0x00000000, .expected = LPM_R0       },
  [TEST_LPM_Z]     = { .name = "LPM_Z",     .value = 0x90040000, .mask = 0x01F00000, .expected = LPM          },
  [TEST_LPM_Z_INC] = { .name = "LPM_Z_INC", .value = 0x90050000, .mask = 0x01F00000, .expected = LPM          },
  [TEST_LSL]       = { .name = "LSL",       .value = 0x0C000000, .mask = 0x03FF0000, .expected = ADD          },
  [TEST_LSR]       = { .name = "LSR",       .value = 0x94060000, .mask = 0x01F00000, .expected = LSR          },
  [TEST_MOV]       = { .name = "MOV",       .value = 0x2C000000, .mask = 0x03FF0000, .expected = MOV          },
  [TEST_MOVW]      = { .name = "MOVW",      .value = 0x01000000, .mask = 0x00FF0000, .expected = MOVW         },
  [TEST_MUL]       = { .name = "MUL",       .value = 0x9C000000, .mask = 0x03FF0000, .expected = MUL          },
  [TEST_MULS]      = { .name = "MULS",      .value = 0x02000000, .mask = 0x00FF0000, .expected = MULS         },
  [TEST_MULSU]     = { .name = "MULSU",     .value = 0x03000000, .mask = 0x00770000, .expected = MULSU        },
  [TEST_NEG]       = { .name = "NEG",       .value = 0x94010000, .mask = 0x01F00000, .expected = NEG          },
  [TEST_NOP]       = { .name = "NOP",       .value = 0x00000000, .mask = 0x00000000, .expected = NOP          },
  [TEST_OR]        = { .name = "OR",        .value = 0x28000000, .mask = 0x03FF0000, .expected = OR           },
  [TEST_ORI]       = { .name = "ORI",       .value = 0x60000000, .mask = 0x0FFF0000, .expected = ORI          },
  [TEST_OUT]       = { .name = "OUT",       .value = 0xB8000000, .mask = 0x07FF0000, .expected = OUT          },
  [TEST_POP]       = { .name = "POP",       .value = 0x900F0000, .mask = 0x01F00000, .expected = POP          },
  [TEST_PUSH]      = { .name = "PUSH",      .value = 0x920F0000, .mask = 0x01F00000, .expected = PUSH         },
  [TEST_RCALL]     = { .name = "RCALL",     .value = 0xD0000000, .mask = 0x0FFF0000, .expected = RCALL        },
  [TEST_RET]       = { .name = "RET",       .value = 0x95080000, .mask = 0x00000000, .expected = RET          },
  [TEST_RETI]      = { .name = "RETI",      .value = 0x95180000, .mask = 0x00000000, .expected = RETI         },
  [TEST_RJMP]      = { .name = "RJMP",      .value = 0xC0000000, .mask = 0x0FFF0000, .expected = RJMP         },
  [TEST_ROL]       = { .name = "ROL",       .value = 0x1C000000, .mask = 0x03FF0000, .expected = ADC          },
  [TEST_ROR]       = { .name = "ROR",       .value = 0x94070000, .mask = 0x01F00000, .expected = ROR          },
  [TEST_SBC]       = { .name = "SBC",       .value = 0x08000000, .mask = 0x03FF0000, .expected = SBC          },
  [TEST_SBCI]      = { .name = "SBCI",      .value = 0x40000000, .mask = 0x0FFF0000, .expected = SBCI         },
  [TEST_SBI]       = { .name = "SBI",       .value = 0x9A000000, .mask = 0x00FF0000, .expected = SBI_CBI      },
  [TEST_SBIC]      = { .name = "SBIC",      .value = 0x99000000, .mask = 0x00FF0000, .expected = SBIC_SBIS    },
  [TEST_SBIS]      = { .name = "SBIS",      .value = 0x9B000000, .mask = 0x00FF0000, .expected = SBIC_SBIS    },
  [TEST_SBIW]      = { .name = "SBIW",      .value = 0x97000000, .mask = 0x00FF0000, .expected = SBIW         },
  [TEST_SBR]       = { .name = "SBR",       .value = 0x60000000, .mask = 0x00FF0000, .expected = ORI          },
  [TEST_SBRC]      = { .name = "SBRC",      .value = 0xFC000000, .mask = 0x01F70000, .expected = SBRC_SBRS    },
  [TEST_SBRS]      = { .name = "SBRS",      .value = 0xFE000000, .mask = 0x01F70000, .expected = SBRC_SBRS    },
  [TEST_SEC]       = { .name = "SEC",       .value = 0x94080000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SEH]       = { .name = "SEH",       .value = 0x94580000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SEI]       = { .name = "SEI",       .value = 0x94780000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SEN]       = { .name = "SEN",       .value = 0x94280000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SER]       = { .name = "SER",       .value = 0xEF0F0000, .mask = 0x00F00000, .expected = LDI          },
  [TEST_SES]       = { .name = "SES",       .value = 0x94480000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SET]       = { .name = "SET",       .value = 0x94680000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SEV]       = { .name = "SEV",       .value = 0x94380000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SEZ]       = { .name = "SEZ",       .value = 0x94180000, .mask = 0x00000000, .expected = BSET         },
  [TEST_SLEEP]     = { .name = "SLEEP",     .value = 0x95880000, .mask = 0x00000000, .expected = SLEEP        },
  [TEST_SPM1_INC]  = { .name = "SPM1_INC",  .value = 0x95E80000, .mask = 0x00000000, .expected = SPM          },
  [TEST_SPM2]      = { .name = "SPM2",      .value = 0x95E80000, .mask = 0x00000000, .expected = SPM          },
  [TEST_SPM2_INC]  = { .name = "SPM2_INC",  .value = 0x95F80000, .mask = 0x00000000, .expected = SPM          },
  [TEST_ST_X]      = { .name = "ST_X",      .value = 0x920C0000, .mask = 0x01F00000, .expected = INDIR_X      },
  [TEST_ST_X_INC]  = { .name = "ST_X_INC",  .value = 0x920D0000, .mask = 0x01F00000, .expected = INDIR_X_INC  },
  [TEST_ST_X_DEC]  = { .name = "ST_X_DEC",  .value = 0x920E0000, .mask = 0x01F00000, .expected = INDIR_X_DEC  },
  [TEST_ST_Y]      = { .name = "ST_Y",      .value = 0x82080000, .mask = 0x01F00000, .expected = INDIR_DP_D16 },
  [TEST_ST_Y_INC]  = { .name = "ST_Y_INC",  .value = 0x92090000, .mask = 0x01F00000, .expected = INDIR_Y_INC  },
  [TEST_ST_Y_DEC]  = { .name = "ST_Y_DEC",  .value = 0x920A0000, .mask = 0x01F00000, .expected = INDIR_Y_DEC  },
  [TEST_STD_Y]     = { .name = "STD_Y",     .value = 0x82080000, .mask = 0x2DF70000, .expected = INDIR_DP_D16 },
  [TEST_ST_Z]      = { .name = "ST_Z",      .value = 0x82000000, .mask = 0x01F00000, .expected = INDIR_DP_D16 },
  [TEST_ST_Z_INC]  = { .name = "ST_Z_INC",  .value = 0x92010000, .mask = 0x01F00000, .expected = INDIR_Z_INC  },
  [TEST_ST_Z_DEC]  = { .name = "ST_Z_DEC",  .value = 0x92020000, .mask = 0x01F00000, .expected = INDIR_Z_DEC  },
  [TEST_STD_Z]     = { .name = "STD_Z",     .value = 0x82000000, .mask = 0x2DF70000, .expected = INDIR_DP_D16 },
  [TEST_STS32]     = { .name = "STS32",     .value = 0x92000000, .mask = 0x01F0FFFF, .expected = STS32        },
  [TEST_STS16]     = { .name = "STS16",     .value = 0xA8000000, .mask = 0x07FF0000, .expected = INDIR_DP_D16 },
  [TEST_SUB]       = { .name = "SUB",       .value = 0x18000000, .mask = 0x03FF0000, .expected = SUB          },
  [TEST_SUBI]      = { .name = "SUBI",      .value = 0x50000000, .mask = 0x0FFF0000, .expected = SUBI         },
  [TEST_SWAP]      = { .name = "SWAP",      .value = 0x94020000, .mask = 0x01F00000, .expected = SWAP         },
  [TEST_TST]       = { .name = "TST",       .value = 0x20000000, .mask = 0x03FF0000, .expected = AND          },
  [TEST_WDR]       = { .name = "WDR",       .value = 0x95A80000, .mask = 0x00000000, .expected = WDR          },
  [TEST_XCH]       = { .name = "XCH",       .value = 0x92040000, .mask = 0x01F00000, .expected = XCH          },
};
static_assert((sizeof(test_opcodes) / sizeof(test_opcodes[0])) == NUM_TEST_OPCODES,
    "Not all tests are in the test_opcodes array");

int times_ran = 0;
int tests_passed = 0;
void test_decode_opcode() {
  for (int i = 0; i < NUM_TEST_OPCODES; i++) {
    times_ran++;
    bool success = false;
    uint32_t oned_value = test_opcodes[i].value |
      (test_opcodes[i].mask & 0xFFFFFFFF);
    OpcodeType result = opcode_decode(oned_value);
    if (result != test_opcodes[i].expected) {
      success = false;
      printf("Test failed for %s with input 0x%08X, got: %u, exptected %u\n",
          test_opcodes[i].name, oned_value,
          result, test_opcodes[i].expected);
    } else {
      success = true;
    }

    uint32_t zeroed_value = test_opcodes[i].value;
    result = opcode_decode(zeroed_value);
    if (result != test_opcodes[i].expected) {
      success = false;
      printf("Test failed for %s with input 0x%08X, got: %u, exptected %u\n",
          test_opcodes[i].name, zeroed_value,
          result, test_opcodes[i].expected);
    } else {
      success = true;
    }
    
    for (int j = 0; j < 100; j++) {
      srand(time(NULL));
      uint32_t random_number = (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
      uint32_t random_value = test_opcodes[i].value |
          (test_opcodes[i].mask & random_number);
      result = opcode_decode(random_value);
      if (result != test_opcodes[i].expected) {
        printf("Test failed for %s with input 0x%08X, got: %u, exptected %u\n",
            test_opcodes[i].name, random_value,
            result, test_opcodes[i].expected);
        success = false;
        break;
      } else {
        success = true;
      }
    }
    
    if (success) {
      tests_passed++;
      printf("Test decode: %s PASSED\n", test_opcodes[i].name);
    } else {
      printf("Test decode: %s FAILED\n", test_opcodes[i].name);
    }
  }
}

int main(void) {
  test_decode_opcode();
  printf("%d out of %d tests passed\n", tests_passed, times_ran);

  return 1;
}
