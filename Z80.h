#pragma once

#include "Types.h"

class Memory;
class GameRom;
union Register
{
	Register(word v) : value(v) {}

	word value;
	struct
	{
		byte hi;
		byte lo;
	};
};

enum FLAG : uint8_t
{
	CARRY           = 0, // 
	ADD_SUBSTRACT   = 1, // Add or Substract instruction
	PARITY_OVERFLOW = 2, // Has even number of bits set OR result doesn't fit in register
	// 3 UNUSED
	HALF_CARRY      = 4, // Substract op: carry from bit 4 to 3 (1 byte), or from 12 to 11 (2 bytes)
	// 55 UNUSED
	ZERO            = 6, // Operation result is zero.
	SIGN            = 7  // 
};

class Z80;
using OPCodeFunc = uint8_t(*)(Z80&);
/*
	
*/
class Z80
{
public:
	enum class InterruptMode : uint8_t
	{
		MODE_0 = 0,
		MODE_1,
		MODE_2
	};

public:
	Z80();
	~Z80();

	bool ReadFlag(FLAG flag);
	word ReadWord();
	byte ReadByte();

	word GetPrefixedHLAddress();
	Register& GetPrefixedHL();

	uint32_t Tick();

	void LoadGame(GameRom& rom);

private:

	void ProcessOPCode(byte opcode, OPCodeFunc[256]);
	void IncrementRefresh();

	void WriteFlag(FLAG flag, bool value);


private:
	static bool HasParity(const byte data);

public:
	Register m_reg_AF; // A - Accumulator. F - Flags
	Register m_reg_BC;
	Register m_reg_DE;
	Register m_reg_HL; // Address registers.

	// Shadow registers aren't modified. They should be swapped with the normal ones.
	Register m_reg_AF_shadow;
	Register m_reg_BC_shadow;
	Register m_reg_DE_shadow;
	Register m_reg_HL_shadow;

	// HL Prefixes: 
	Register m_reg_IX;
	Register m_reg_IY;

	word m_program_counter; // points to next opcode
	word m_stack_pointer; // next address to store variable in stack.
	byte m_reg_refresh; // increments with each opcode.
	byte m_reg_interrupt; //

private:
	uint32_t m_cycle_count; // cycles that needs the opcode.

	Memory* m_memory;

	byte m_current_prefix;
	bool m_halt;

	bool m_IFF1;
	bool m_IFF2;
	bool m_after_EI;

	InterruptMode m_interrupt_mode;

// opcodes declaration.
public:
	void ADD(byte& acc, byte add);
	void ADD(word add); // The register used will depend on the current prefix.
	void ADD_HL(byte& acc);
	void ADC(byte& acc, byte add); // Same as ADD but using carry
	void ADC(word add);            // Same as ADD but using carry
	void ADC_HL(byte& acc);
	void ADC_HL(word num);
	void AND(byte data);
	void AND_HL();
	void BIT(uint8_t bit, byte& reg);
	void BIT_HL(uint8_t bit);
	void CALL();
	bool CALL(bool cond);
	void CCF();
	void CP(byte sub);
	void CP_HL();
	void CPD();
	bool CPDR();
	void CPI();
	bool CPIR();
	void CPL();
	void DAA();
	void DI();
	void DEC(byte& reg);
	void DEC(Register& reg);
	void DEC(word& reg);
	void DEC_HL();
	bool DJNZ();
	void EI();
	void EX(Register& reg, Register& shd);
	void EX(Register& reg);
	void EX_SPHL();
	void EXX();
	void HALT();	
	void IM0();
	void IM1();
	void IM2();
	void IN(byte& in, const byte reg);
	void IND();
	bool INDR();
	void INC(byte& reg);
	void INC(Register& reg);
	void INC(word& reg);
	void INC_HL();
	void INI();
	bool INIR();
	void JP();
	void JP(word add);
	void JP(bool cond);
	void JR();
	bool JR(bool cond);
	void LD(byte& reg, byte data);
	void LD(byte& reg, word add);
	void LD(word  add, byte reg);
	void LD_DDNN(Register& reg);
	void LD_DDNN(word& reg);
	void LD_DDNN(byte& reg);
	void LD_NNDD(Register& reg);
	void LD_NNDD(word& reg);
	void LD_NNDD(byte data);
	void LD_HL_N();
	void LD_SP_HL();
	void LDD();
	bool LDDR();
	void LDI();
	bool LDIR();
	void LD_AR();
	void LD_AI();
	void NEG();
	void NOP();
	void OR(byte data);
	void OR_HL();
	void OUT(const byte reg, byte& out);
	void OUTD();
	void OUTI();
	bool OUTR();
	bool OTIR();
	void POP(Register& reg);
	void PUSH(const Register reg);
	void RES(uint8_t bit, byte& reg);
	void RES_HL(uint8_t bit);
	void RET();
	bool RET(bool cond);
	void RETI();
	void RETN();
	void RL(byte& reg);
	void RL_HL();
	void RLA();
	void RLC(byte& reg);
	void RLC_HL();
	void RLCA();
	void RLD();
	void RR(byte& reg);
	void RR_HL();
	void RRA();
	void RRC(byte& reg);
	void RRC_HL();
	void RRCA();
	void RRD();
	void RST(byte data);
	void SBC(byte& acc, byte add);	// Same as SUB but using carry
	void SBC(word add);	            // Same as SUB but using carry
	void SBC_HL(byte& acc);
	void SBC_HL(word add);
	void SCF();
	void SET(uint8_t bit, byte& reg);
	void SET_HL(uint8_t bit);
	void SLA(byte& reg);
	void SLA_HL();
	void SRA(byte& reg);
	void SRA_HL();
	void SRL(byte& reg);
	void SRL_HL();
	void SUB(byte& acc, byte sub);
	void SUB(word sub);
	void SUB_HL(byte& acc);
	void XOR(byte data);
	void XOR_HL();
	// Special codes
	void CB();
	void DD();
	void ED();
	void FD();

	void UNUSED();
};

