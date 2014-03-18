
#include "i8086sim.h"


KInstructionID K8086VM::InsID[] =
{
	{"MOV: Register/Memory to/from Register",		{0x88, 0x00}, {0xFC, 0x00}, 2},	//100010dw|mod reg r/m
	{"MOV: Immediate to Register/Memory",			{0xC6, 0x00}, {0xFE, 0x38}, 3},	//1100011w|mod 000 r/m|data|data if w=1
	{"MOV: Immediate to Register",					{0xB0, 0x00}, {0xF0, 0x00}, 2},	//1011wreg|data|data if w=1
	{"MOV: Memory to Accumulator",					{0xA0, 0x00}, {0xFE, 0x00}, 3},	//1010000w|addr-low|addr-high
	{"MOV: Accumulator to Memory",					{0xA2, 0x00}, {0xFE, 0x00}, 3},	//1010001w|addr-low|addr-high
	{"MOV: Register/Memory to Segment Register",	{0x8E, 0x00}, {0xFF, 0x00}, 2},	//10001110|mod reg r/m
	{"MOV: Segment Register to Register/Memory",	{0x8C, 0x00}, {0xFF, 0x00}, 2},	//10001100|mod reg r/m
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_MOV},

	{"PUSH: Register/Memory",	{0xFF, 0x30}, {0xFF, 0x38}, 2},	//11111111|mod 110 r/m
	{"PUSH: Register",			{0x50, 0x00}, {0xF8, 0x00}, 1},	//01010reg
	{"PUSH: Segment Register",	{0x06, 0x00}, {0xE7, 0x00}, 1},	//000reg110
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_PUSH},

	{"POP: Register/Memory",	{0x8F, 0x00}, {0xFF, 0x38}, 2},	//10001111|mod 000 r/m
	{"POP: Register",				{0x58, 0x00}, {0xF8, 0x00}, 1},	//01011reg
	{"POP: Segment Register",	{0x07, 0x00}, {0xE7, 0x00}, 1},	//000reg111
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_POP},

	{"XCHG: Register/Memory with Register",	{0x86, 0x00}, {0xFE, 0x00}, 2},	//1000011w|mod reg r/m
	{"XCHG: Register with Accumulator",			{0x90, 0x00}, {0xF8, 0x00}, 1},	//10010reg
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_XCHG},

	{"XLAT: Translate Byte to AL", {0xD7, 0x00}, {0xFF, 0x00}, 1},	//11010111
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_XLAT},

	{"LEA: Load EA to Register",	{0x8D, 0x00}, {0xFF, 0x00}, 2},	//10001101|mod reg r/m
	{"LDS: Load Pointer to DS",	{0xC5, 0x00}, {0xFF, 0x00}, 2},	//11000101|mod reg r/m
	{"LES: Load Pointer to ES",	{0xC4, 0x00}, {0xFF, 0x00}, 2},	//11000100|mod reg r/m
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_LEA_LDS_LES},

	{"LAHF: Load AH with Flags",	{0x9F, 0x00}, {0xFF, 0x00}, 1},	//10011111
	{"SAHF: Store AH into Flags",	{0x9E, 0x00}, {0xFF, 0x00}, 1},	//10011110
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_LAHF_SAHF},

	{"PUSHF: Push Flags",	{0x9C, 0x00}, {0xFF, 0x00}, 1},	//10011100
	{"POPF: Pop Flags",		{0x9D, 0x00}, {0xFF, 0x00}, 1},	//10011101
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_PUSHF_POPF},

	{"ADD: Register/Memory with Register to Either",	{0x00, 0x00}, {0xFC, 0x00}, 2},	//000000dw|mod reg r/m
	{"ADD: Immediate to Register/Memory",					{0x80, 0x00}, {0xFC, 0x38}, 3},	//100000sw|mod 000 r/m|data|data if sw=01
	{"ADD: Immediate to Accumulator",						{0x04, 0x00}, {0xFE, 0x00}, 2},	//0000010w|data|data if w=1
	{"ADC: Register/Memory with Register to Either",	{0x10, 0x00}, {0xFC, 0x00}, 2},	//000100dw|mod reg r/m
	{"ADC: Immediate to Register/Memory",					{0x80, 0x10}, {0xFC, 0x38}, 3},	//100000sw|mod 010 r/m|data|data if sw=01
	{"ADC: Immediate to Accumulator",						{0x14, 0x00}, {0xFE, 0x00}, 2},	//0001010w|data|data if w=1
	{"INC: Register/Memory",	{0xFE, 0x00}, {0xFE, 0x38}, 2},	//1111111w|mod 000 r/m
	{"INC: Register",				{0x40, 0x00}, {0xF8, 0x00}, 1},	//01000reg
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_ADD_ADC_INC},

	{"SUB: Register/Memory and Register to Either",	{0x28, 0x00}, {0xFC, 0x00}, 2},	//001010dw|mod reg r/m
	{"SUB: Immediate from Register/Memory",			{0x80, 0x28}, {0xFC, 0x38}, 3},	//100000sw|mod 101 r/m|data|data if sw=01
	{"SUB: Immediate from Accumulator",					{0x2C, 0x00}, {0xFE, 0x00}, 2},	//0010110w|data|data if w=1
	{"SSB: Register/Memory and Register to Either",	{0x18, 0x00}, {0xFC, 0x00}, 2},	//000110dw|mod reg r/m
	{"SSB: Immediate from Register/Memory",			{0x80, 0x18}, {0xFC, 0x38}, 3},	//100000sw|mod 011 r/m|data|data if sw=01
	{"SSB: Immediate from Accumulator",					{0x0E, 0x00}, {0xFE, 0x00}, 2},	//0000111w|data|data if w=1
	{"DEC: Register/Memory",	{0xFE, 0x08}, {0xFE, 0x38}, 2},	//1111111w|mod 001 r/m
	{"DEC: Register",				{0x48, 0x00}, {0xF8, 0x00}, 1},	//01001reg
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_SUB_SSB_DEC},

	{"NEG: Change sign", {0xF6, 0x18}, {0xFE, 0x38}, 2},	//1111011w|mod 011 r/m
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_NEG},

	{"CMP: Register/Memory and Register",		{0x38, 0x00}, {0xFC, 0x00}, 2},	//001110dw|mod reg r/m
	{"CMP: Immediate with Register/Memory",	{0x80, 0x38}, {0xFC, 0x38}, 3},	//100000sw|mod 111 r/m|data|data if sw=01
	{"CMP: Immediate with Accumulator",			{0x3C, 0x00}, {0xFE, 0x00}, 2},	//0011110w|data|data if w=1
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_CMP},

	{"AAA: ASCII Adjust for Add",				{0x37, 0x00}, {0xFF, 0x00}, 1},	//00110111
	{"DAA: Decimal Adjust for Add",			{0x27, 0x00}, {0xFF, 0x00}, 1},	//00100111
	{"AAS: ASCII Adjust for Subtract",		{0x3F, 0x00}, {0xFF, 0x00}, 1},	//00111111
	{"DAS: Decimal Adjust for Subtract",	{0x2F, 0x00}, {0xFF, 0x00}, 1},	//00101111
	{"AAM: ASCII Adjust for Multiply",		{0xD4, 0x0A}, {0xFF, 0xFF}, 2},	//11010100|00001010
	{"AAD: ASCII Adjust for Divide",			{0xD5, 0x0A}, {0xFF, 0xFF}, 2},	//11010101|00001010
	{"CBW: Convert Byte to Word",				{0x98, 0x00}, {0xFF, 0x00}, 1},	//10011000
	{"CWD: Convert Word to Double Word",	{0x99, 0x00}, {0xFF, 0x00}, 1},	//10011001
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_AAA_DAA_AAS_DAS_AAM_AAD_CBW_CWD},

	{"MUL: Multiply (Unsigned)",	{0xF6, 0x20}, {0xFE, 0x38}, 2},	//1111011w|mod 100 r/m
	{"IMUL: Multiply (Signed)",	{0xF6, 0x28}, {0xFE, 0x38}, 2},	//1111011w|mod 101 r/m
	{"DIV: Divide (Unsigned)",		{0xF6, 0x30}, {0xFE, 0x38}, 2},	//1111011w|mod 110 r/m
	{"IDIV: Divide (Signed)",		{0xF6, 0x38}, {0xFE, 0x38}, 2},	//1111011w|mod 111 r/m
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_MUL_IMUL_DIV_IDIV},

	{"NOT: Invert",										{0xF6, 0x10}, {0xFE, 0x38}, 2},	//1111011w|mod 010 r/m
	{"SHL/SAL: Shift Logical/Arithmetic Left",	{0xD0, 0x20}, {0xFC, 0x38}, 2},	//110100vw|mod 100 r/m
	{"SHR: Shift Logical Right",						{0xD0, 0x28}, {0xFC, 0x38}, 2},	//110100vw|mod 101 r/m
	{"SAR: Shift Arithmetic Right",					{0xD0, 0x38}, {0xFC, 0x38}, 2},	//110100vw|mod 111 r/m
	{"ROL: Rotate Left",									{0xD0, 0x00}, {0xFC, 0x38}, 2},	//110100vw|mod 000 r/m
	{"ROR: Rotate Right",								{0xD0, 0x08}, {0xFC, 0x38}, 2},	//110100vw|mod 001 r/m
	{"RCL: Rotate Through Carry Flag Left",		{0xD0, 0x10}, {0xFC, 0x38}, 2},	//110100vw|mod 010 r/m
	{"RCR: Rotate Through Carry Right",				{0xD0, 0x18}, {0xFC, 0x38}, 2},	//110100vw|mod 011 r/m
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_NOT_SHL_SAL_SHR_SAR_ROL_ROR_RCL_RCR},

	{"TEST: Register/Memory and Register",		{0x84, 0x00}, {0xFE, 0x00}, 2},	//1000010w|mod reg r/m
	{"TEST: Immediate with Register/Memory",	{0xF6, 0x00}, {0xFE, 0x38}, 3},	//1111011w|mod 000 r/m|data|data if w=1
	{"TEST: Immediate with Accumulator",		{0xA8, 0x00}, {0xFE, 0x00}, 2},	//1010100w|data|data if w=1
	{"AND: Register/Memory and Register",		{0x20, 0x00}, {0xFC, 0x00}, 2},	//001000dw|mod reg r/m
	{"AND: Immediate with Register/Memory",	{0x80, 0x20}, {0xFE, 0x38}, 3},	//1000000w|mod 100 r/m|data|data if w=1
	{"AND: Immediate with Accumulator",			{0x24, 0x00}, {0xFE, 0x00}, 2},	//0010010w|data|data if w=1
	{"OR: Register/Memory and Register",		{0x08, 0x00}, {0xFC, 0x00}, 2},	//000010dw|mod reg r/m
	{"OR: Immediate with Register/Memory",		{0x80, 0x08}, {0xFE, 0x38}, 3},	//1000000w|mod 001 r/m|data|data if w=1
	{"OR: Immediate with Accumulator",			{0x0C, 0x00}, {0xFE, 0x00}, 2},	//0000110w|data|data if w=1
	{"XOR: Register/Memory and Register",		{0x30, 0x00}, {0xFC, 0x00}, 2},	//001100dw|mod reg r/m
	{"XOR: Immediate with Register/Memory",	{0x80, 0x30}, {0xFE, 0x38}, 3},	//1000000w|mod 110 r/m|data|data if w=1
	{"XOR: Immediate with Accumulator",			{0x34, 0x00}, {0xFE, 0x00}, 2},	//0011010w|data|data if w=1
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_TEST_AND_OR_XOR},

	{"REP: Repeat",	{0xF2, 0xA0}, {0xFE, 0xF0}, 2},	//1111001z|1010 ins w
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_REP},

	{"CALL: Direct within Segment",		{0xE8, 0x00}, {0xFF, 0x00}, 3},	//11101000|disp-low|disp-high
	{"CALL: Indirect within Segment",	{0xFF, 0x10}, {0xFF, 0x38}, 2},	//11111111|mod 010 r/m
	{"CALL: Direct Intersegment",			{0x9A, 0x00}, {0xFF, 0x00}, 5},	//10011010|offset-low|offset-high|seg-low|seg-high
	{"CALL: Indirect Intersegment",		{0xFF, 0x18}, {0xFF, 0x38}, 2},	//11111111|mod 011 r/m
	{"RET: Within Segment",									{0xC3, 0x00}, {0xFF, 0x00}, 1},	//11000011
	{"RET: Within Segment Adding Immediate to SP",	{0xC2, 0x00}, {0xFF, 0x00}, 3},	//11000010|data-low|data-high
	{"RET: Intersegment",									{0xCB, 0x00}, {0xFF, 0x00}, 1},	//11001011
	{"RET: Intersegment Adding Immediate to SP",		{0xCA, 0x00}, {0xFF, 0x00}, 3},	//11001010|data-low|data-high
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_CALL_RET},

	{"JMP: Direct within Segment",			{0xE9, 0x00}, {0xFF, 0x00}, 3},	//11101001|disp-low|disp-high
	{"JMP: Direct within Segment-Short",	{0xEB, 0x00}, {0xFF, 0x00}, 2},	//11101011|disp
	{"JMP: Indirect within Segment",			{0xFF, 0x20}, {0xFF, 0x38}, 2},	//11111111|mod 100 r/m
	{"JMP: Direct Intersegment",				{0xEA, 0x00}, {0xFF, 0x00}, 5},	//11101010|offset-low|offset-high|seg-low|seg-high
	{"JMP: Indirect Intersegment",			{0xFF, 0x28}, {0xFF, 0x38}, 2},	//11111111|mod 101 r/m
	{"JCXZ: Jump if CX is Zero",				{0xE3, 0x00}, {0xFF, 0x00}, 2},	//11100011|disp
	{"JE: Jump if Equal",						{0x74, 0x00}, {0xFF, 0x00}, 2},	//01110100|disp
	{"JNE: Jump if Not Equal",					{0x75, 0x00}, {0xFF, 0x00}, 2},	//01110101|disp
	{"JL: Jump if Less",							{0x7C, 0x00}, {0xFF, 0x00}, 2},	//01111100|disp
	{"JLE: Jump if Less or Equal",			{0x7E, 0x00}, {0xFF, 0x00}, 2},	//01111110|disp
	{"JG: Jump if Greater",						{0x7F, 0x00}, {0xFF, 0x00}, 2},	//01111111|disp
	{"JGE: Jump if Greater or Equal",		{0x7D, 0x00}, {0xFF, 0x00}, 2},	//01111101|disp
	{"JB: Jump if Below",						{0x72, 0x00}, {0xFF, 0x00}, 2},	//01110010|disp
	{"JBE: Jump if Below or Equal",			{0x76, 0x00}, {0xFF, 0x00}, 2},	//01110110|disp
	{"JA: Jump if Above",						{0x77, 0x00}, {0xFF, 0x00}, 2},	//01110111|disp
	{"JAE: Jump if Above or Equal",			{0x73, 0x00}, {0xFF, 0x00}, 2},	//01110011|disp
	{"JO: Jump if Overflow",					{0x70, 0x00}, {0xFF, 0x00}, 2},	//01110000|disp
	{"JNO: Jump if No Overflow",				{0x71, 0x00}, {0xFF, 0x00}, 2},	//01110001|disp
	{"JS: Jump if Signed",						{0x78, 0x00}, {0xFF, 0x00}, 2},	//01111000|disp
	{"JNS: Jump if Not Signed",				{0x79, 0x00}, {0xFF, 0x00}, 2},	//01111001|disp
	{"JPE: Jump if Parity Even",				{0x7A, 0x00}, {0xFF, 0x00}, 2},	//01111010|disp
	{"JPO: Jump if Parity Odd",				{0x7B, 0x00}, {0xFF, 0x00}, 2},	//01111011|disp
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_JMP_JZ_JNZ_JL_JLE_JG_JGE_JB_JBE_JA_JAE_JO_JNO_JS_JNS_JPO_JPE_JCXZ},

	{"LOOP: Loop CX Times",				{0xE2, 0x00}, {0xFF, 0x00}, 2},	//11100010|disp
	{"LOOPE: Loop While Equal",		{0xE1, 0x00}, {0xFF, 0x00}, 2},	//11100001|disp
	{"LOOPNE: Loop While Not Equal",	{0xE0, 0x00}, {0xFF, 0x00}, 2},	//11100000|disp
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_LOOP_LOOPE_LOOPNE},

	{"CLC: Clear Carry",			{0xF8, 0x00}, {0xFF, 0x00}, 1},	//11111000
	{"CMC: Complement Carry",	{0xF5, 0x00}, {0xFF, 0x00}, 1},	//11110101
	{"STC: Set Carry",			{0xF9, 0x00}, {0xFF, 0x00}, 1},	//11111001
	{"CLD: Clear Direction",	{0xFC, 0x00}, {0xFF, 0x00}, 1},	//11111100
	{"STD: Set Direction",		{0xFD, 0x00}, {0xFF, 0x00}, 1},	//11111101
	{"CLI: Clear Interrupt",	{0xFA, 0x00}, {0xFF, 0x00}, 1},	//11111010
	{"STI: Set Interrupt",		{0xFB, 0x00}, {0xFF, 0x00}, 1},	//11111011
	{"HLT: Halt",					{0xF4, 0x00}, {0xFF, 0x00}, 1},	//11110100
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_CLC_CMC_STC_CLD_STD_CLI_STI_HLT},

	{"INT: Type specified",	{0xCD, 0x00}, {0xFF, 0x00}, 2},	//11001101|type
	{"INT: Type 3",			{0xCC, 0x00}, {0xFF, 0x00}, 1},	//11001100
	{NULL, {0xFF, 0xFF}, {0x00, 0x00}, 0, K8086VM::Execute_INT},
};

uint K8086VM::InsIDCount = _countof(K8086VM::InsID);
