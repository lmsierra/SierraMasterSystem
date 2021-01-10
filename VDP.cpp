#include "VDP.h"
#include <assert.h>
#include <iostream>

const uint32_t RES_W = 256;
const uint32_t RES_H = 192;

VDP::VDP(Z80* cpu) :
	m_cpu           (cpu),
	m_command_word  (0x0000),
	m_is_first_byte (false),
	m_status_flags  (0x00),
	m_read_buffer   (0x00),
	m_line_mode     (LINE_MODE::DEFAULT),
	m_pal           (true)
{
	m_VRam      = (byte*)calloc(0x4000, sizeof(byte));
	m_CRam      = (byte*)calloc(32,     sizeof(byte));
	m_registers = (byte*)calloc(16,     sizeof(byte));

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

void VDP::Tick(uint32_t cycles)
{
	const uint8_t height_lines = static_cast<uint8_t>(m_line_mode);

}

void VDP::SetPal(bool is_pal)
{
	m_pal = is_pal;
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
				m_line_mode = GetLineMode();
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
