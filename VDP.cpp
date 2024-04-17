#include "VDP.h"
#include <assert.h>
#include <iostream>

constexpr VLineFormat NTSC_256x192   = VLineFormat(192, 24, 3, 3, 13, 27);
constexpr VLineFormat NTSC_256x224   = VLineFormat(224,  8, 3, 3, 13, 11);
constexpr VLineFormat PAL_256x192    = VLineFormat(192, 48, 3, 3, 13, 54);
constexpr VLineFormat PAL_256x224    = VLineFormat(224, 32, 3, 3, 13, 38);
constexpr VLineFormat PAL_256x240    = VLineFormat(240, 24, 3, 3, 13, 30);

constexpr uint8_t VIDEO_MAX_CYCLES = 228;

VDP::VDP(Z80& cpu) :
    m_cpu             (cpu),
    m_command_word    (0x0000),
    m_is_first_byte   (false),
    m_status_flags    (0x00),
    m_read_buffer     (0x00),
    m_line_mode       (LINE_MODE::DEFAULT),
    m_pal             (true),
    m_h_counter       (0),
    m_v_counter       (0),
    m_cycle_count	  (0),
    m_line_format	  (VLineFormat()),
    m_format_dirt     (true),
    m_current_line    (0)
{
    m_VRam          = (byte*)calloc(0x4000,                 sizeof(byte));
    m_CRam          = (byte*)calloc(32,                     sizeof(byte));
    m_registers     = (byte*)calloc(16,                     sizeof(byte));
    m_buffer        = (byte*)calloc(MAX_WIDTH * MAX_HEIGHT, sizeof(byte));

    /* https://segaretro.org/Sega_Master_System_VDP_documentation_(2002-11-12) */
    m_registers[0]  = 0b00110110; // Mode Control No. 1
    m_registers[1]  = 0b10000000; // Mode Control No. 2
    m_registers[2]  = 0b11111111; // Name Table Base Address
    m_registers[3]  = 0b00000111; // Color Table Base Address
    m_registers[4]  = 0b00000111; // Background Pattern Generator Base Address
    m_registers[5]  = 0b11111111; // Sprite Attribute Table Base Address
    m_registers[6]  = 0b11111111; // Sprite Pattern Generator Base Address
    m_registers[7]  = 0b11111111; // Overscan/Backdrop Color
    m_registers[8]  = 0b11111111; // Background X Scroll
    m_registers[9]  = 0b11111111; // Background Y Scroll
    m_registers[10] = 0b11111111; // Line counter
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
    
    // Max cycles reached
    m_cycle_count += cycles;
    if (m_cycle_count > VIDEO_MAX_CYCLES)
        return false;

    if (IsDisplayVisible())
    {

    }
    else
    {

    }

    // Scan line and that stuff.
    const uint16_t display_begin = line_format.top_blanking + line_format.vertical_blanking + line_format.top_border;
    const uint16_t display_end   = display_begin + line_format.active_display;
    const uint16_t line          = m_current_line - display_begin;

    if (line < display_end)
    {
        if (line == 0)
        {
            if (m_registers[0] & (1<<3))
            {
                // Request Interrupt (on CPU)
            
            }
        }
        else
        {

        }
    }
    else if (line - line_format.active_display == 1 && m_registers[1] & (1<<4))
    {
        // Request Interrupt 2
        
    }

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
                m_line_mode   = GetLineMode();
                m_format_dirt = true;
            }
            break;
        }
        default: 
            break;
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
        m_line_format = FindLineFormat();
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

byte VDP::GetVCounter() const
{
    if (m_pal)
    {
        switch (m_line_mode)
        {
        case LINE_MODE::DEFAULT: 
            return m_v_counter > 0xF2 ? m_v_counter - (0xF2 - 0xBA + 1) : m_v_counter;
        
        case LINE_MODE::MODE_224: 	
            if ((m_v_counter - 0xFF) > 0x02)
            {
                return m_v_counter - (0xFF - 0xCA + 1);
            }
            else if (m_v_counter > 0xFF) // 0x00-0x02
            {
                return m_v_counter - (0xFF + 1);
            }

        case LINE_MODE::MODE_240: 
            if ((m_v_counter - 0xFF) > 0x0A) // 
            {
                return m_v_counter - (0xFF - 0xD2 + 1);
            }
            else if (m_v_counter > 0xFF) // 0x00-0x0A
            {
                return m_v_counter - (0xFF + 1);
            }
        }
    }
    else
    {
        switch (m_line_mode)
        {
        case LINE_MODE::DEFAULT:  return m_v_counter > 0xDA ? m_v_counter - (0xDA - 0xD5 + 1) : m_v_counter;
        case LINE_MODE::MODE_224: return m_v_counter > 0xEA ? m_v_counter - (0xEA - 0xE5 + 1) : m_v_counter;
        case LINE_MODE::MODE_240: assert(false && "LINE_MODE::MODE_240 - Unsupported mode for NTSC"); //Drop
        }
    }

    return static_cast<uint8_t>(m_v_counter);
}

bool VDP::IsDisplayVisible() const
{
    return m_registers[1] && (1 << 5);
}

SCREEN_MODE VDP::GetScreenMode() const
{
    SCREEN_MODE result = SCREEN_MODE::GRAPHIC_I;
    
    const byte m1 = m_registers[1] & (1 << 4);
    const byte m2 = m_registers[0] & (1 << 1);
    const byte m3 = m_registers[1] & (1 << 3);
    const byte m4 = m_registers[0] & (1 << 2);

    if (m4)
    {
        result = m1 ? SCREEN_MODE::INVALID_TEXT : SCREEN_MODE::MODE_4;
    }
    else
    {
        const byte mode_byte = m3 | m2 | m1;
        result = static_cast<SCREEN_MODE>(mode_byte);
    }

    return result;
}

bool VDP::IsVerticalScrollActive() const
{
    return !(m_registers[0] ^ (1 << 7));
}

bool VDP::IsHorizontalScrollActive() const
{
    return !(m_registers[0] & (1 << 6));
}

bool VDP::ShouldUseOverscanColor() const
{
    return m_registers[0] & (1 << 5);
}

bool VDP::IsLineInterruptEnabled() const
{
    return m_registers[0] & (1 << 4);
}

bool VDP::ShouldShiftSprites() const
{
    return m_registers[0] & (1 << 3);
}

bool VDP::IsMonochromeDisplay() const
{
    return m_registers[0] & 1;
}

bool VDP::IsFrameInterruptEnabled() const
{
    return m_registers[1] & (1 << 6);
}

bool VDP::AreSpritesDoubleSized() const
{
    return m_registers[1] & 1;
}

SPRITE_SIZE VDP::GetSpriteSize() const
{
    const bool sprite_flag = m_registers[1] & (1 << 1);
    if (GetScreenMode() == SCREEN_MODE::MODE_4)
    {
        return sprite_flag ? SPRITE_SIZE::SIZE_8x16 : SPRITE_SIZE::SIZE_8x8;
    }
    else
    {
        return sprite_flag ? SPRITE_SIZE::SIZE_16x16 : SPRITE_SIZE::SIZE_8x8;
    }
}

void VDP::SetSpriteCollision()
{
    m_status_flags |= (1 << 4);
}

void VDP::SetSpriteOverflow()
{
    m_status_flags |= (1 << 5);
}
