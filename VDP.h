#pragma once

#include "Types.h"

class Z80;
class VDP
{
public:
	VDP(Z80* cpu);
	~VDP();

	byte ReadRam      (word address);
	void WriteRam     (word address, byte data);
	void WriteAddress (byte data);

private:
	word GetAddressRegister() const;

	/*
	 Code value         Actions taken

		0               A byte of VRAM is read from the location defined by the
						address register and is stored in the read buffer. The
						address register is incremented by one. Writes to the
						data port go to VRAM.
		1               Writes to the data port go to VRAM.
		2               This value signifies a VDP register write, explained
			            below. Writes to the data port go to VRAM.
		3               Writes to the data port go to CRAM.
	*/
	byte GetCodeRegister() const;

	void IncrementAddressRegister();

private:
	Z80*  m_cpu;
	byte* m_VRam;
	byte* m_CRam;
	byte* m_registers;

	/* 14 bits: address. 2 MSB: code regiter */
	word m_command_word;
	bool m_is_first_byte;

	/*
		BIT 7   = Frame Interrupt Pending
		BIT 6   = Sprite Overflow
		BIT 5   = Sprite Collision
		BIT 4-0 = Unused
	*/
	byte m_status_flags;

	byte m_read_buffer;

};

