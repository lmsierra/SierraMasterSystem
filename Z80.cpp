#include "Z80.h"
#include "Memory.h"
#include "Z80Instructions/Z80Instructions.h"

#include <iostream>
#include <assert.h>

Z80::Z80() :
	m_memory		  (new Memory()),
	m_reg_AF		  (0x0040),
	m_reg_BC		  (0x0000),
	m_reg_DE		  (0x0000),
	m_reg_HL		  (0x0000),
	m_reg_AF_shadow   (0x0000),
	m_reg_BC_shadow   (0x0000),
	m_reg_DE_shadow   (0x0000),
	m_reg_HL_shadow   (0x0000),
	m_reg_IX		  (0xFFFF),
	m_reg_IY          (0xFFFF),
	m_program_counter (0x0000),
	m_stack_pointer   (0xDFF0),
	m_cycle_count     (0x0000),
	m_current_prefix  (0x0000),
	m_reg_interrupt   (0x0000),
	m_reg_refresh	  (0x0000),
	m_halt			  (false),
	m_IFF1            (false),
	m_IFF2            (false),
	m_after_EI        (false),
	m_interrupt_mode  (InterruptMode::MODE_0)
{

}

Z80::~Z80()
{

}

uint32_t Z80::Tick()
{
	byte opcode = ReadByte();
	ProcessOPCode(opcode, Z80Instructions::s_opcode_funcs);
	
	std::cout << "Running OPCode: " << std::hex << static_cast<int>(opcode) << "\n";
	return m_cycle_count;
}

void Z80::ProcessOPCode(byte opcode, OPCodeFunc funcs [256])
{
	assert(funcs != nullptr && "Array of functions passed cannot be null");
	
	IncrementRefresh();

	m_cycle_count = funcs[opcode](*this);
}

void Z80::IncrementRefresh()
{
	// once the lower 6 bits or the r register reaches 127 
	// then the lower 6 bits are all set to 0
	m_reg_refresh = ((m_reg_refresh + 1) & 0x7F) | (m_reg_refresh & 0x80);
}

bool Z80::HasParity(const byte data)
{
	uint8_t count = 0;
	byte aux = data;
	while (aux != 0)
	{
		count += data & 1;
		aux = aux >> 1;
	}

	return count % 2 == 0;
}

word Z80::ReadWord()
{
	const word result = m_memory->ReadMemory(m_program_counter + 1) << 8 | m_memory->ReadMemory(m_program_counter);
	m_program_counter += 2;
	return result;
}

byte Z80::ReadByte()
{
	const byte result = m_memory->ReadMemory(m_program_counter);
	++m_program_counter;
	return result;
}

bool Z80::ReadFlag(FLAG flag)
{
	return m_reg_AF.lo & (1 << flag);
}

void Z80::WriteFlag(FLAG flag, bool value)
{
	m_reg_AF.lo = value ? (m_reg_AF.lo | (1<<flag)) : (m_reg_AF.lo & ~(1<<flag));
}

Register& Z80::GetPrefixedHL()
{
	switch (m_current_prefix)
	{
		case 0xDD: return m_reg_IX;
		case 0xFD: return m_reg_IY;
		default:   return m_reg_HL;
	}
}


word Z80::GetPrefixedHLAddress()
{
	switch (m_current_prefix)
	{
	case 0xDD:
	{
		const byte offset = m_memory->ReadMemory(m_program_counter);
		++m_program_counter;
		return m_reg_IX.value + offset;
	}
	case 0xED:
	{
		const byte offset = m_memory->ReadMemory(m_program_counter);
		++m_program_counter;
		return m_reg_IY.value + offset;
	}
	default:
		return m_reg_HL.value;
	}
}

void Z80::LoadGame(GameRom& rom)
{
	m_memory->LoadRom(rom);
}

////////////////////////////////////////////////////////////////////////////////////////

/*
	OPCode Operations:
*/

void Z80::ADD(byte& acc, byte add)
{
	WriteFlag(ADD_SUBSTRACT, 0);
	
	// For calculating the flags we need to store the results in a bigger container. 
	const word result     = acc + add;
	const bool zero       = (result & 0xFF) == 0;
	const bool carry      = (result & 0xFF) == 0;
	const bool half_carry = (acc & 0x0F) + (add & 0x0F) >= 0x10;
	const bool sign       = (result & 0x80) != 0;
	const bool overflow   = ((acc & 0x80) == (add & 0x80)) && ((add & 0x80) != (result & 0x80));

	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);

	acc += add;
}

void Z80::ADD(word add)
{
	Register& reg = GetPrefixedHL();

	WriteFlag(ADD_SUBSTRACT, 0);

	const uint32_t result     = reg.value + add;
	const bool     zero       = (result & 0xFFFF) == 0;
	const bool     carry      = (result > 0xFFFF);
	const bool     half_carry = ((reg.value & 0x0FFF) + (add & 0x0FFF) >= 0x0FFF);
	const bool     sign       = (result & (0x80)) != 0;
	const bool     overflow   = ((reg.value & 0x8000) == (add & 0x8000)) && ((reg.value & 0x8000) != (result & 0x8000));
	
	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);
	
	reg.value += add;
}

void Z80::ADD_HL(byte& acc)
{
	ADD(acc, m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::ADC(byte& acc, byte add)
{
	const uint8_t carry = ReadFlag(CARRY) ? 1 : 0;
	ADD(acc, add + carry);
}

void Z80::ADC(word add)
{
	const uint8_t carry = ReadFlag(CARRY) ? 1 : 0;
	ADD(add + carry);
}

void Z80::ADC_HL(byte& acc)
{
	ADC(acc, m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::ADC_HL(word num)
{
	const uint32_t result     = m_reg_HL.value + num + ReadFlag(CARRY);
	const bool     zero       = (result & 0xFFFF) == 0;
	const bool     carry      = (result > 0xFFFF);
	const bool     half_carry = ((m_reg_HL.value & 0x0FFF) + (num & 0x0FFF) >= 0x0FFF);
	const bool     sign       = (result & (0x80)) != 0;
	const bool     overflow   = ((m_reg_HL.value & 0x80) != (num & 0x80)) && ((num & 0x80) == (result & 0x80));

	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);
	WriteFlag(ADD_SUBSTRACT,   0);

	m_reg_HL.value = result;
}

void Z80::AND(byte data)
{
	m_reg_AF.hi = data & m_reg_AF.hi;

	WriteFlag(SIGN,            m_reg_AF.hi & 0x80);
	WriteFlag(ZERO,            m_reg_AF.hi == 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(CARRY,           0);
	WriteFlag(PARITY_OVERFLOW, HasParity(m_reg_AF.hi));
}

void Z80::AND_HL()
{
	AND(m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::BIT(uint8_t bit, byte& reg)
{
	const bool bit_val = reg & (1 << bit);
	
	WriteFlag(ZERO,          bit_val);
	WriteFlag(ADD_SUBSTRACT, 0);
	WriteFlag(HALF_CARRY,    1);
}

void Z80::BIT_HL(uint8_t bit)
{
	const word address = GetPrefixedHLAddress();
	const byte value   = m_memory->ReadMemory(address) & (1 << bit);
	WriteFlag(ZERO,          value);
	WriteFlag(ADD_SUBSTRACT, 0);
	WriteFlag(HALF_CARRY,    1);
}

void Z80::CALL()
{
	const word address = m_program_counter;
	const byte lo      = m_memory->ReadMemory(address);
	const byte hi      = m_memory->ReadMemory(address + 1);
	
	m_program_counter += 2;
	
	m_memory->WriteMemory(m_stack_pointer - 1, (m_program_counter & 0xFF00) >> 8);	
	m_memory->WriteMemory(m_stack_pointer - 2, m_program_counter & 0X00FF);
	m_stack_pointer -= 2;

	m_program_counter = lo + (hi << 8);
}

void Z80::CALL(bool cond)
{
	if (cond)
		CALL();
}

void Z80::CCF()
{
	WriteFlag(CARRY, !ReadFlag(CARRY));
	WriteFlag(ADD_SUBSTRACT, 0);
}

/*
	CP is the SUB(A, data) operator except it doesn't update the register, only updates the flags
*/
void Z80::CP(byte sub)
{
	const word result     = m_reg_AF.hi - sub;
	const bool half_carry = (m_reg_AF.hi & 0x0F) + (sub & 0x0F) >= 0x10;
	const bool overflow   = ((m_reg_AF.hi & 0x80) != (sub & 0x80)) && ((sub & 0x80) == (result & 0x80));

	WriteFlag(ADD_SUBSTRACT,   1);
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(CARRY,		   result & 0xFF00);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            result & 0x80);
	WriteFlag(PARITY_OVERFLOW, overflow);
}

void Z80::CP_HL()
{
	CP(m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::CPD()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);
	CP(value);

	--m_reg_HL.value;
	--m_reg_BC.value;
}

void Z80::CPDR()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);
	CP(value);

	--m_reg_HL.value;
	--m_reg_BC.value;

	if (m_reg_BC.value - 1 == 0 || value != m_reg_AF.hi)
		return;

	m_program_counter -= 2;
}

void Z80::CPI()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);

	++m_reg_HL.value;
	--m_reg_BC.value;
	
	WriteFlag(ZERO,            value == m_reg_AF.hi);
	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
	WriteFlag(SIGN,            value & 0x80);
	WriteFlag(HALF_CARRY,      value & (1 << 3));
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::CPIR()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);

	++m_reg_HL.value;
	--m_reg_BC.value;
	
	WriteFlag(ZERO,            value == m_reg_AF.hi);
	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
	WriteFlag(SIGN,            value & 0x80);
	WriteFlag(HALF_CARRY,      value & (1 << 3));
	WriteFlag(ADD_SUBSTRACT,   0);

	if (m_reg_BC.value == 0 && value == m_reg_AF.hi)
	{
		m_program_counter -= 2;
	}
}

void Z80::CPL()
{
	m_reg_AF.hi = ~m_reg_AF.hi;

	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::DAA()
{
	const bool add_sub_flag    = ReadFlag(ADD_SUBSTRACT);
	const bool carry_flag      = ReadFlag(CARRY);
	const bool half_carry_flag = ReadFlag(HALF_CARRY);
	
	byte acc       = m_reg_AF.hi;
	bool new_carry = 0;

	if (!add_sub_flag) 
	{  // addition: adjust if carry and half occurred or if result is out of bounds
		if (carry_flag || acc > 0x99)
		{ 
			acc += 0x60;
			new_carry = 1;
		}
		if (half_carry_flag || (acc & 0x0f) > 0x09) 
		{
			acc += 0x6;
		}
	}
	else
	{  // subtraction: adjust if carry and half occurred
		if (carry_flag) 
		{
			acc -= 0x60;
		}
		if (half_carry_flag) 
		{
			acc -= 0x6;
		}
	}

	WriteFlag(CARRY, new_carry);
	WriteFlag(ZERO, acc == 0);
	WriteFlag(HALF_CARRY, 0);
	
	m_reg_AF.hi = acc;
}

void Z80::DI()
{
	m_IFF1 = false;
	m_IFF2 = false;
}

void Z80::DEC(byte& reg)
{
	SUB(reg, 1);
}

void Z80::DEC(Register& reg)
{
	--reg.value;
}

void Z80::DEC(word& reg)
{
	--reg;
}

void Z80::DEC_HL()
{
	const word address = GetPrefixedHLAddress();
	const byte result = m_memory->ReadMemory(address - 1);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY, result & (1 << 7));
	WriteFlag(ZERO,  result == 0);
	WriteFlag(SIGN,  result & 0x80);
}

void Z80::DJNZ()
{
	--m_reg_BC.hi;
	if (m_reg_BC.hi == 0)
	{
		const byte dis = m_memory->ReadMemory(m_program_counter);
		m_program_counter += dis;
	}
	else
	{
		++m_program_counter;
	}
}

void Z80::EI()
{
	m_IFF1     = true;
	m_IFF2     = true;
	m_after_EI = true;
}

void Z80::EX(Register& reg, Register& shd)
{
	const word tmp = reg.value;
	reg.value = shd.value;
	shd.value = tmp;
}

void Z80::EX(Register& reg)
{
	word address    = m_stack_pointer;
	m_stack_pointer = reg.value;
	reg.lo          = m_memory->ReadMemory(address);
	reg.hi          = m_memory->ReadMemory(address + 1);
}

void Z80::EX_SPHL()
{
	Register&  reg = GetPrefixedHL();
	const byte lo  = reg.lo;
	const byte hi  = reg.hi;
	
	reg.lo = m_memory->ReadMemory(m_stack_pointer);
	reg.hi = m_memory->ReadMemory(m_stack_pointer + 1);

	m_memory->WriteMemory(m_stack_pointer, lo);
	m_memory->WriteMemory(m_stack_pointer + 1, hi);
}

void Z80::EXX()
{
	word tmp              = m_reg_BC.value;
	m_reg_BC.value        = m_reg_BC_shadow.value;
	m_reg_BC_shadow.value = tmp;

	tmp                   = m_reg_DE.value;
	m_reg_DE.value        = m_reg_DE_shadow.value;
	m_reg_DE_shadow.value = tmp;

	tmp                   = m_reg_HL.value;
	m_reg_HL.value        = m_reg_HL_shadow.value;
	m_reg_HL_shadow.value = tmp;
}

void Z80::HALT()
{
	m_halt = true;
}

void Z80::IM0()
{
	m_interrupt_mode = InterruptMode::MODE_0;
}

void Z80::IM1()
{
	m_interrupt_mode = InterruptMode::MODE_1;
}

void Z80::IM2()
{
	m_interrupt_mode = InterruptMode::MODE_2;
}

void Z80::IN(byte& in, const byte reg)
{
	// static_assert(false, "To implement");
}

void Z80::IND()
{
	// static_assert(false, "To implement");
}

void Z80::INDR()
{
	// static_assert(false, "To implement");
}

void Z80::INC(byte& reg)
{
	ADD(reg, 1);
}

void Z80::INC(Register& reg)
{
	++reg.value;
}

void Z80::INC(word& reg)
{
	++reg;
}

void Z80::INC_HL()
{
	const word address = GetPrefixedHLAddress();
	const byte result  = m_memory->ReadMemory(address + 1);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY, result & (1 << 7));
	WriteFlag(ZERO, result == 0);
	WriteFlag(SIGN, result & 0x80);
}

void Z80::INI()
{
	// static_assert(false, "Falta implementar");
}

void Z80::INIR()
{
	// static_assert(false, "Falta implementar");
}

void Z80::JP()
{
	const word address = m_program_counter;
	const byte lo      = m_memory->ReadMemory(address);
	const byte hi      = m_memory->ReadMemory(address + 1);
	m_program_counter  = lo + (hi << 8);
}

void Z80::JP(word address)
{
	m_program_counter = address;
}

void Z80::JP(bool cond)
{
	if (cond)
		JP();
	else
		m_program_counter += 2;
}

void Z80::JR()
{
	const word value = m_program_counter;
	m_program_counter = value + 1 + (m_memory->ReadMemory(value));
}

void Z80::JR(bool cond)
{
	if (cond)
		JR();
	else
		++m_program_counter;
}

void Z80::LD(byte& reg, byte data)
{
	reg = data;
}

void Z80::LD(byte& reg, word add)
{
	reg = m_memory->ReadMemory(add);
}

void Z80::LD(word address, byte reg)
{
	m_memory->WriteMemory(address, reg);
}

void Z80::LD_DDNN(Register& reg)
{
	const word address = ReadWord();
	reg.value = m_memory->ReadMemory(address + 1) << 8 | m_memory->ReadMemory(address);
}

void Z80::LD_DDNN(word& reg)
{
	const word address = ReadWord();
	reg = m_memory->ReadMemory(address + 1) << 8 | m_memory->ReadMemory(address);
}

void Z80::LD_DDNN(byte& reg)
{
	const word address = ReadWord();
	reg = m_memory->ReadMemory(address);
}

void Z80::LD_NNDD(Register& reg)
{
	const word address = ReadWord();
	m_memory->WriteMemory(address, reg.lo);
	m_memory->WriteMemory(address+1, reg.hi);
}

void Z80::LD_NNDD(byte data)
{
	const word address = ReadWord();
	m_memory->WriteMemory(address, data);
}

void Z80::LD_HL_N()
{
	const word address = GetPrefixedHLAddress();
	const byte n	   = m_memory->ReadMemory(m_program_counter + 1);

	m_memory->WriteMemory(address, n);
	++m_program_counter;
}

void Z80::LD_SP_HL()
{
	m_stack_pointer = GetPrefixedHL().value;
}

void Z80::LDD()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);
	m_memory->WriteMemory(m_reg_DE.value, value);

	--m_reg_HL.value;
	--m_reg_DE.value;
	--m_reg_BC.value;

	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::LDDR()
{
	const byte value = m_memory->ReadMemory(m_reg_HL.value);
	m_memory->WriteMemory(m_reg_DE.value, value);

	--m_reg_HL.value;
	--m_reg_DE.value;
	--m_reg_BC.value;
	
	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);

	if (m_reg_BC.value == 0)
		return;

	m_program_counter -= 2;
}

void Z80::LDI()
{
	const byte result = m_memory->ReadMemory(m_reg_HL.value);
	m_memory->WriteMemory(m_reg_DE.value, result);
	
	++m_reg_DE.value;
	++m_reg_HL.value;
	--m_reg_BC.value;

	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
}

void Z80::LDIR()
{
	const byte result = m_memory->ReadMemory(m_reg_HL.value);
	m_memory->WriteMemory(m_reg_DE.value, result);

	++m_reg_DE.value;
	++m_reg_HL.value;
	--m_reg_BC.value;

	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(PARITY_OVERFLOW, m_reg_BC.value - 1 != 0);
	
	if (m_reg_BC.value == 0)
		return;

	m_program_counter -= 2;
}

void Z80::LD_AR()
{
	// LD A,R
	const byte value = m_reg_refresh;
	LD(m_reg_AF.hi, value);

	WriteFlag(SIGN,            value & 0x80);
	WriteFlag(ZERO,            value == 0);
	WriteFlag(PARITY_OVERFLOW, m_IFF2);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::LD_AI()
{
	 // LD A,I
	const byte value = m_reg_interrupt;
	LD(m_reg_AF.hi, value);

	WriteFlag(SIGN, value & 0x80);
	WriteFlag(ZERO, value == 0);
	WriteFlag(HALF_CARRY, 0);
	WriteFlag(ADD_SUBSTRACT, 0);
	WriteFlag(PARITY_OVERFLOW, m_IFF2);
}

void Z80::NEG()
{ 
	const byte result = 0 - m_reg_AF.hi;
	
	WriteFlag(SIGN,            result & 0x80);
	WriteFlag(ZERO,            result == 0);
	WriteFlag(HALF_CARRY,      result & (1<<3));
	WriteFlag(PARITY_OVERFLOW, m_reg_AF.hi == 0x80);
	WriteFlag(ADD_SUBSTRACT,   1);
	WriteFlag(CARRY,		   m_reg_AF.hi != 0);

	m_reg_AF.hi = result;
}

void Z80::NOP()
{
	// Do nothing.
}

void Z80::OR(byte data)
{
	m_reg_AF.hi = data | m_reg_AF.hi;

	WriteFlag(SIGN,            m_reg_AF.hi & 0x80);
	WriteFlag(ZERO,            m_reg_AF.hi == 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(CARRY,           0);
	WriteFlag(PARITY_OVERFLOW, HasParity(m_reg_AF.hi));
}

void Z80::OR_HL()
{
	OR(m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::OUT(const byte reg, byte& out)
{
	// static_assert(false, "To implement");
}

void Z80::OUTD()
{
	// static_assert(false, "To implement");
}

void Z80::OUTI()
{
	// static_assert(false, "To implement");
}

void Z80::OUTR()
{
	// static_assert(false, "To implement");
}

void Z80::OTIR()
{
	// static_assert(false, "To implement");
}

void Z80::POP(Register& reg)
{
	byte lo = m_memory->ReadMemory(m_stack_pointer);
	byte hi = m_memory->ReadMemory(m_stack_pointer + 1);
	m_stack_pointer += 2;
	reg.value = lo + (hi << 8);
}

void Z80::PUSH(const Register reg)
{
	const word address = m_stack_pointer;
	m_memory->WriteMemory(address,     reg.hi);
	m_memory->WriteMemory(address - 1, reg.lo);
	m_stack_pointer -= 2;
}

void Z80::RES(uint8_t bit, byte& reg)
{
	reg = reg & ~(1 << bit);
}

void Z80::RES_HL(uint8_t bit)
{
	const word address = GetPrefixedHLAddress();
	const byte result  = ~(1 << m_memory->ReadMemory(address));
	m_memory->WriteMemory(address, result);
}

void Z80::RET()
{
	byte lo = m_memory->ReadMemory(m_stack_pointer);
	byte hi = m_memory->ReadMemory(m_stack_pointer + 1);
	m_stack_pointer += 2;
	m_program_counter = lo + (hi << 8);
}

void Z80::RET(bool cond)
{
	if (cond)
		RET();
	else
		++m_program_counter;
}

void Z80::RETI()
{
	RET();
	m_IFF1 = m_IFF2;
}

void Z80::RETN()
{
	RET();
	m_IFF1 = m_IFF2;
}

void Z80::RL(byte& reg)
{
	const bool carry = reg & (1 << 7);
	reg = (reg << 1) + ReadFlag(CARRY);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,            reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RL_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & (1 << 7);
	
	result = (result << 1) + ReadFlag(CARRY);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RLA()
{
	const bool carry = (m_reg_AF.hi & (1 << 7));
	m_reg_AF.hi      = (m_reg_AF.hi << 1) + ReadFlag(CARRY);

	WriteFlag(CARRY,         carry);
	WriteFlag(HALF_CARRY,    0);
	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::RLC(byte& reg)
{
	const bool carry = reg & (1 << 7);
	reg = (reg << 1) + carry;

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,            reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RLC_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result = m_memory->ReadMemory(address);
	const bool carry  = result & (1 << 7);
	
	result = (result << 1) + carry;
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RLCA()
{
	const bool carry = m_reg_AF.hi & 0x80;
	m_reg_AF.hi      = (m_reg_AF.hi << 1) + carry;

	WriteFlag(CARRY,         carry);
	WriteFlag(HALF_CARRY,    0);
	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::RLD()
{
	const word address = m_reg_HL.value;
	const byte value   = m_memory->ReadMemory(address);
	const byte result  = (m_reg_AF.hi & 0xF0) | ((value >> 4) & 0x0F);

	m_memory->WriteMemory(address, ((value << 4) & 0xF0) | (m_reg_AF.hi & 0x0F));
	m_reg_AF.hi = result;

	WriteFlag(SIGN,            result & 0x80);
	WriteFlag(ZERO,            result == 0);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RR(byte& reg)
{
	const bool carry = reg & 1;
	reg = (reg >> 1) + (ReadFlag(CARRY) << 7);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,            reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RR_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & 1;
	
	result = (result >> 1) + (ReadFlag(CARRY) << 7);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RRA()
{
	const bool carry = m_reg_AF.hi & 1;
	m_reg_AF.hi      = (m_reg_AF.hi >> 1) + (ReadFlag(CARRY) << 7);

	WriteFlag(CARRY,         carry);
	WriteFlag(HALF_CARRY,    0);
	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::RRC(byte& reg)
{
	const bool carry = reg & 1;
	reg = (reg >> 1) + (carry << 7);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,            reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RRC_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & 1;

	result = (result >> 1) + (carry >> 7);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(SIGN,            result & 0x80);
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RRCA()
{
	const bool carry = m_reg_AF.hi & 1;
	m_reg_AF.hi      = (m_reg_AF.hi >> 1) + (carry << 7);

	WriteFlag(CARRY,         carry);
	WriteFlag(HALF_CARRY,    0);
	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::RRD()
{
	const word address = m_reg_HL.value;
	const byte value   = m_memory->ReadMemory(address);
	const byte result  = (m_reg_AF.hi & 0XF0) | (value & 0x0F);
	
	m_memory->WriteMemory(address, ((m_reg_AF.hi << 4) & 0xF0) | ((value >> 4) & 0x0F));
	m_reg_AF.hi = result;

	WriteFlag(SIGN,            result & 0x80);
	WriteFlag(ZERO,            result == 0);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::RST(byte data)
{
	const word address = m_stack_pointer;
	m_memory->WriteMemory(address, m_program_counter & 0x00FF);
	m_memory->WriteMemory(address - 1, m_program_counter >> 8);
	m_stack_pointer -= 2;
	m_program_counter = data;
}

void Z80::SBC(byte& acc, byte sub)
{
	const uint8_t carry = ReadFlag(CARRY) ? 1 : 0;
	SUB(acc, sub + carry);
}

void Z80::SBC(word sub)
{
	const uint8_t carry = ReadFlag(CARRY) ? 1 : 0;
	SUB(sub + carry);
}

void Z80::SBC_HL(byte& acc)
{
	SBC(acc, m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::SBC_HL(word num)
{
	const uint32_t result     = m_reg_HL.value - num - ReadFlag(CARRY);
	const bool     zero       = (result & 0xFFFF) == 0;
	const bool     carry      = (result > 0xFFFF);
	const bool     half_carry = ((m_reg_HL.value & 0x0FFF) + (num & 0x0FFF) >= 0x0FFF);
	const bool     sign       = (result & (0x80)) != 0;
	const bool     overflow   = ((m_reg_HL.value & 0x80) != (num & 0x80)) && ((num & 0x80) == (result & 0x80));

	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);
	WriteFlag(ADD_SUBSTRACT,   1);

	m_reg_HL.value = result;
}

void Z80::SCF()
{
	WriteFlag(CARRY,         1);
	WriteFlag(HALF_CARRY,    0);
	WriteFlag(ADD_SUBSTRACT, 0);
}

void Z80::SET(uint8_t bit, byte& reg)
{
	reg = reg | (1 << bit);
}

void Z80::SET_HL(uint8_t bit)
{
	const word address = GetPrefixedHLAddress();
	const byte result  = m_memory->ReadMemory(address) | (1 << bit);
	m_memory->WriteMemory(address, result);
}

void Z80::SLA(byte& reg)
{
	const bool carry = reg & (1 << 7);
	reg = reg << 1;

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,            reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::SLA_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & (1 << 7);
	
	result = result << 1;
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::SRA(byte& reg)
{
	const bool carry = reg & 1 ;
	const byte bit7  = reg & (1 << 7);
	reg = bit7 + (reg >> 1);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(SIGN,			   reg & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::SRA_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & 1;
	const byte bit7    = result & (1 << 7);

	result = bit7 + (result >> 1);
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::SRL(byte& reg)
{
	const bool carry = reg & 1 ;
	reg = reg >> 1;

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(reg));
	WriteFlag(ZERO,            reg == 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(SIGN,            0);

}

void Z80::SRL_HL()
{
	const word address = GetPrefixedHLAddress();
	byte       result  = m_memory->ReadMemory(address);
	const bool carry   = result & 1;
	
	result = result >> 1;
	m_memory->WriteMemory(address, result);

	WriteFlag(CARRY,           carry);
	WriteFlag(PARITY_OVERFLOW, HasParity(result));
	WriteFlag(ZERO,			   result == 0);
	WriteFlag(SIGN,			   result & 0x80);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
}

void Z80::SUB(byte& acc, byte sub)
{
	WriteFlag(ADD_SUBSTRACT, 1);

	const word result     = acc - sub;
	const bool zero       = (result & 0xFF) == 0;
	const bool carry      = (result & 0xFF) == 0;
	const bool half_carry = (acc & 0x0F) + (sub & 0x0F) >= 0x10;
	const bool sign       = (result & 0x80) != 0;
	const bool overflow   = ((acc & 0x80) != (sub & 0x80)) && ((sub & 0x80) == (result & 0x80));

	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);
	WriteFlag(ADD_SUBSTRACT,   1);

	acc -= sub;
}

void Z80::SUB(word sub)
{
	Register& reg = GetPrefixedHL();
	
	const uint32_t result     = reg.value - sub;
	const bool     zero       = (result & 0xFFFF) == 0;
	const bool     carry      = (result > 0xFFFF);
	const bool     half_carry = ((reg.value & 0x0FFF) + (sub & 0x0FFF) >= 0x0FFF);
	const bool     sign       = (result & (0x80)) != 0;
	const bool     overflow   = ((reg.value & 0x80) != (sub & 0x80)) && ((sub & 0x80) == (result & 0x80));

	WriteFlag(ZERO,            zero);
	WriteFlag(CARRY,           carry);
	WriteFlag(HALF_CARRY,      half_carry);
	WriteFlag(SIGN,            sign);
	WriteFlag(PARITY_OVERFLOW, overflow);
	WriteFlag(ADD_SUBSTRACT,   1);

	reg.value -= sub;
}

void Z80::SUB_HL(byte& acc)
{
	SUB(acc, m_memory->ReadMemory(GetPrefixedHLAddress()));
}

void Z80::XOR(byte data)
{
	m_reg_AF.hi = data ^ m_reg_AF.hi;

	WriteFlag(SIGN,            m_reg_AF.hi & 0x80);
	WriteFlag(ZERO,            m_reg_AF.hi == 0);
	WriteFlag(HALF_CARRY,      0);
	WriteFlag(ADD_SUBSTRACT,   0);
	WriteFlag(CARRY,           0);
	WriteFlag(PARITY_OVERFLOW, HasParity(m_reg_AF.hi));
}

void Z80::XOR_HL()
{
	XOR(m_memory->ReadMemory(GetPrefixedHLAddress()));
}

// Special codes
void Z80::CB()
{

}

void Z80::DD()
{

}

void Z80::ED()
{

}

void Z80::FD()
{

}

void Z80::UNUSED()
{
	assert(false && "This opcode is not meant to be used");
}
