#pragma once

#include "Types.h"

class Z80;
class VDP
{
	enum class LINE_MODE : uint8_t
	{
		DEFAULT  = 192,
	    MODE_224 = 224,
		MODE_240 = 240
	};

public:
	VDP(Z80* cpu);
	~VDP();

	void Tick		  (uint32_t cycles);
	void SetPal       (bool is_pal);

private:
	void WriteData    (byte data);
	void WriteAddress (byte data);

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

	LINE_MODE GetLineMode() const;
	void IncrementAddressRegister();

private:
	Z80*      m_cpu;
	byte*     m_VRam;
	byte*     m_CRam;
	byte*     m_registers;

	/* 14 bits: address. 2 MSB: code regiter */
	word      m_command_word;
	bool      m_is_first_byte;

	/*
		BIT 7   = Frame Interrupt Pending
		BIT 6   = Sprite Overflow
		BIT 5   = Sprite Collision
		BIT 4-0 = Unused
	*/
	byte      m_status_flags;

	byte      m_read_buffer;
	LINE_MODE m_line_mode;
	bool      m_pal;
};

