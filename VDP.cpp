#include "VDP.h"
#include <assert.h>
#include <iostream>

constexpr uint32_t    MAX_WIDTH    = 256;
constexpr uint32_t    MAX_HEIGHT   = 192;

constexpr VLineFormat NTSC_256x192   = VLineFormat(192, 24, 3, 3, 13, 27);
constexpr VLineFormat NTSC_256x224   = VLineFormat(224,  8, 3, 3, 13, 11);
constexpr VLineFormat PAL_256x192    = VLineFormat(192, 48, 3, 3, 13, 54);
constexpr VLineFormat PAL_256x224    = VLineFormat(224, 32, 3, 3, 13, 38);
constexpr VLineFormat PAL_256x240    = VLineFormat(240, 24, 3, 3, 13, 30);

VDP::VDP(Z80* cpu) :
	m_cpu            (cpu),
	m_command_word   (0x0000),
	m_is_first_byte  (false),
	m_status_flags   (0x00),
	m_read_buffer    (0x00),
	m_line_mode      (LINE_MODE::DEFAULT),
	m_pal            (true),
	m_h_counter      (0),
	m_v_counter      (0),
	m_cycle_count	 (0),
	m_line_format	 (VLineFormat()),
	m_format_dirt    (true),
	m_current_line   (0)
{
	m_VRam          = (byte*)calloc(0x4000,                 sizeof(byte));
	m_CRam          = (byte*)calloc(32,                     sizeof(byte));
	m_registers     = (byte*)calloc(16,                     sizeof(byte));
	m_buffer        = (byte*)calloc(MAX_WIDTH * MAX_HEIGHT, sizeof(byte));
	/* https://segaretro.org/Sega_Master_System_VDP_documentation_(2002-11-12) */
	m_registers[0]  = 0b00110110;
	m_registers[1]  = 0b10000000;
	m_registers[2]  = 0b11111111;
	m_registers[3]  = 0b00000111;
	m_registers[4]  = 0b11111111;
	m_registers[5]  = 0b11111111;
	m_registers[10] = 0b11111111;
}

VDP::~VDP()
{
	free(m_VRam);
	free(m_CRam);
	free(m_registers);
}

bool VDP::Tick(uint32_t cycles)
{
	const uint8_t      height_lines = static_cast<uint8_t>(m_line_mode);
	const VLineFormat& line_format  = GetCurrentLineFormat();
	
	bool vblank = false;
	
	m_cycle_count += cycles;
	
	// Scan line and that stuff.


	// m_cycle_count -= m_system_info.cycles_per_line;

	return vblank;
}


void VDP::SetPal(bool is_pal)
{
	m_pal         = is_pal;
	m_format_dirt = true;
}

void VDP::WriteData(byte data)
{
	m_is_first_byte = true;
	m_read_buffer   = data;

	switch (GetCodeRegister())
	{
	case 0: // DROP
	case 1: // DROP
	case 2:
		m_VRam[GetAddressRegister()] = data;
		break;
	case 3:
		m_CRam[GetAddressRegister() & 0x1f] = data;
		break;
	default:
		break;
	}
}

void VDP::WriteAddress(byte data)
{
	if (m_is_first_byte)
	{
		m_command_word  = (m_command_word & 0xff00) | data;
		m_is_first_byte = false;
	}
	else
	{
		m_is_first_byte = true;
		m_command_word  = (m_command_word & 0x00ff) | (data << 8);

		switch (GetCodeRegister())
		{
		case 0:
			m_read_buffer = m_VRam[GetAddressRegister()];
			IncrementAddressRegister();
			break;
		
		case 2:
		{
			const byte reg = data & 0xf;

			assert(reg < 11 && "Register must have a value between 0 and 10");

			m_registers[reg] = GetAddressRegister() && 0x00ff;
			if (reg < 2)
			{
				m_line_mode      = GetLineMode();
				m_format_dirt = true;
			}
			break;
		}
		default: break;
		}
	}
}

word VDP::GetAddressRegister() const
{
	return m_command_word & 0x3fff;
}

byte VDP::GetCodeRegister() const
{
	return m_command_word >> 14;
}

VDP::LINE_MODE VDP::GetLineMode() const
{
	if ((m_registers[0] & 0x06) == 0x06 && (m_registers[1] & 0x10) == 0x10)
		return LINE_MODE::MODE_224;
	else if ((m_registers[0] & 0x06) == 0x06 && (m_registers[1] & 0x4) == 0x4)
		return LINE_MODE::MODE_240;
	else
		return LINE_MODE::DEFAULT;
}

void VDP::IncrementAddressRegister()
{
	if (m_command_word & 0x3ff)
		++m_command_word;
	else
		m_command_word = 0xC000;
}

const VLineFormat& VDP::GetCurrentLineFormat()
{
	if (m_format_dirt)
	{
		m_line_format    = FindLineFormat();
		m_format_dirt = false;
	}

	return m_line_format;
}

VLineFormat VDP::FindLineFormat() const
{
	if (m_pal)
	{
		switch (m_line_mode)
		{
		case LINE_MODE::DEFAULT:  return PAL_256x192;
		case LINE_MODE::MODE_224: return PAL_256x224;
		case LINE_MODE::MODE_240: return PAL_256x240;
		default: assert(false && "Unknown format.");
		}
	}
	else
	{
		switch (m_line_mode)
		{
		case LINE_MODE::DEFAULT:  return NTSC_256x192;
		case LINE_MODE::MODE_224: return NTSC_256x224;
		case LINE_MODE::MODE_240: assert(false && "240 line mode not supported for NTSC"); break;
		default: assert(false && "Unknown format.");
		}
	}

	return VLineFormat();
}
