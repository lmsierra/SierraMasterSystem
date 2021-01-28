#pragma once

#include "Types.h"

struct VLineFormat
{
	VLineFormat() : 
		active_display    (0),
		bottom_border     (0),
		bottom_blanking   (0),
		vertical_blanking (0),
		top_blanking      (0),
		top_border        (0)
	{
	
	}

	constexpr VLineFormat(uint8_t _active_display,
					      uint8_t _bottom_border,
					      uint8_t _bottom_blanking,
					      uint8_t _vertical_blanking,
					      uint8_t _top_blanking,
					      uint8_t _top_border) : 
											active_display    (_active_display),
											bottom_border     (_bottom_border),
											bottom_blanking   (_bottom_blanking),
											vertical_blanking (_vertical_blanking),
											top_blanking      (_top_blanking),
											top_border        (_top_border)
	{

	}

	uint8_t active_display;
	uint8_t bottom_border;
	uint8_t bottom_blanking;
	uint8_t vertical_blanking;
	uint8_t top_blanking;
	uint8_t top_border;
};

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

	bool               Tick		                (uint32_t cycles);
	void               SetPal                   (bool is_pal);

private:
	void               WriteData                (byte data);
	void               WriteAddress             (byte data);
	word               GetAddressRegister       () const;
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
	byte               GetCodeRegister          () const;
	LINE_MODE          GetLineMode              () const;
	void               IncrementAddressRegister ();
	const VLineFormat& GetCurrentLineFormat     ();
	bool			   IsDisplayEnabled         () const;
	byte			   GetVCounter              () const;

private:
	VLineFormat FindLineFormat () const;


private:
	Z80*        m_cpu;
	byte*       m_VRam;
	byte*       m_CRam;
	byte*       m_registers;
	byte*       m_buffer;
	/* 14 bits: address. 2 MSB: code regiter */
	word        m_command_word;
	bool        m_is_first_byte;
	/*
		BIT 7   = Frame Interrupt Pending
		BIT 6   = Sprite Overflow
		BIT 5   = Sprite Collision
		BIT 4-0 = Unused
	*/
	byte        m_status_flags;
	byte        m_read_buffer;
	LINE_MODE   m_line_mode;
	bool        m_pal;
	uint16_t    m_h_counter; // 16 bits, only 9 used
	uint16_t    m_v_counter;
	uint32_t    m_cycle_count;
	VLineFormat m_line_format;
	bool	    m_format_dirt;
	uint16_t    m_current_line;
	bool        m_display_enabled;
};
