#include "VDP.h"
#include <assert.h>
#include <iostream>

constexpr VLineFormat NTSC_256x192   = VLineFormat(192, 24, 3, 3, 13, 27);
constexpr VLineFormat NTSC_256x224   = VLineFormat(224,  8, 3, 3, 13, 11);
constexpr VLineFormat PAL_256x192    = VLineFormat(192, 48, 3, 3, 13, 54);
constexpr VLineFormat PAL_256x224    = VLineFormat(224, 32, 3, 3, 13, 38);
constexpr VLineFormat PAL_256x240    = VLineFormat(240, 24, 3, 3, 13, 30);

// Two palettes can be used. Both contains 16 colors and stored in the VRAM (separated by 16)
constexpr uint8_t SECONDARY_COLOR_PALETTE_OFFSET = 16;

// RGB 
constexpr uint32_t NUM_COLOR_COMPONENTS = 3;

// Width * height * 3 color components
constexpr uint32_t FRAME_BUFFER_SIZE = VDP::MAX_WIDTH * VDP::MAX_HEIGHT * NUM_COLOR_COMPONENTS;

VDP::VDP() :
    m_command_word      (0x0000),
    m_is_first_byte     (false),
    m_status_flags      (0x00),
    m_read_buffer       (0x00),
    m_line_mode         (LINE_MODE::DEFAULT),
    m_pal               (true),
    m_h_counter         (0),
    m_v_counter         (0),
    m_cycle_count	    (0),
    m_line_format	    (VLineFormat()),
    m_format_dirt       (true),
    m_current_line      (0),
    m_request_interrupt (false)
{
    m_VRam          = (byte*)calloc(0x4000,            sizeof(byte));
    m_CRam          = (byte*)calloc(32,                sizeof(byte));
    m_registers     = (byte*)calloc(16,                sizeof(byte));
    m_frame_buffer  = (byte*)calloc(FRAME_BUFFER_SIZE, sizeof(byte));

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
    m_request_interrupt = IsInterruptRequested();

    const uint8_t      height_lines = static_cast<uint8_t>(m_line_mode);
    const VLineFormat& line_format  = GetCurrentLineFormat();
    
    bool vblank = false;
    
    // Max cycles reached
    m_cycle_count += cycles;
   
    // @TODO: is this right?
    // Using 342 cycles as indicated in documentation for h counter
    // As it runs as X2 the Z80, use 342*2 = 684 instead
   
    // const bool ReadNextLine = ((m_h_counter + m_cycle_count) > m_cycles_per_line);
    const bool read_next_line = ((m_h_counter + m_cycle_count) > 684);

    m_h_counter = (m_h_counter + cycles) % (684 + 1);

    // vertical

    if (read_next_line)
    {

        ScanLine(m_current_line);

        // Update line
        m_current_line = (m_current_line + 1) % m_lines_per_frame;

        vblank = m_current_line == GetCurrentLineFormat().active_display;
    }

    /*

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
    */


    return vblank;
}

void VDP::ScanLine(uint32_t line)
{
    const VLineFormat line_format = GetCurrentLineFormat();

    const uint16_t display_begin = line_format.top_blanking + line_format.vertical_blanking + line_format.top_border;
    const uint16_t display_end = display_begin + line_format.active_display;

    if (IsDisplayVisible())
    {
        // ClearScreen(line);
        RenderBackground(line);
    }
    else
    {
        if (line < GetCurrentLineFormat().active_display)
        {
            ClearScreen(line);
        }
    }
}

void VDP::ClearScreen(uint32_t line)
{
    const uint32_t line_start = line * VDP::MAX_WIDTH * 3;

    for (uint32_t i = 0; i < VDP::MAX_WIDTH ; ++i)
    {
        m_frame_buffer[line_start + i * 3]     = 255;
        m_frame_buffer[line_start + i * 3 + 1] = 0;
        m_frame_buffer[line_start + i * 3 + 2] = 255;
    }
}

void VDP::RenderBackground(uint32_t line)
{
    // Only render on valid display
    if (line >= GetCurrentLineFormat().active_display)
    {
        return;
    }

    // get the pixel at which the line starts
    const uint32_t starting_line_pixel = line * VDP::MAX_WIDTH;

    const uint16_t table_name = GetBackgroundTableName();

    // Horizontal scroll
    // If bit #6 of VDP register $00 is set, horizontal scrolling will be fixed at zero for scanlines zero through 15
    const bool is_horizontal_scrolling = line <= 15 && !IsHorizontalScrollActive();
    // Whole shorizontal scroll
    const byte scroll_x = is_horizontal_scrolling ? m_registers[8] : 0;

    // Vertical scroll
    const byte scroll_y = m_registers[9];
    
    const byte mod = IsBckgTableNameExtended() ? 255 : 224;
    uint32_t tile_y = line + scroll_y % mod;
    
    byte starting_row = scroll_y >> 3;
    byte fine_scroll_y = scroll_y & 0b00000111;

    const bool use_overscan_color = ShouldUseOverscanColor();

    uint8_t pixel_color = 0;

    // iterate each pixel horizontally
    for (uint32_t position_x = 0; position_x < VDP::MAX_WIDTH; ++position_x)
    {
        // pixel in screen we have to draw
        const uint32_t screen_pixel_index = starting_line_pixel + position_x;

        if (use_overscan_color && position_x < 8)
        {
            pixel_color = GetOverscanColor();
            pixel_color += SECONDARY_COLOR_PALETTE_OFFSET; // Use second color palette
        }
        else
        {
            // If bit 7 of register $00 is set, the vertical scroll value will be fixed to zero when columns 24 to 31 are rendered. 
            constexpr uint8_t MIN_INDEX_FOR_V_SCROLL = 24 * 8; // (24 * 8 = 192)

            const bool is_vertical_scrolling_disabled = position_x >= MIN_INDEX_FOR_V_SCROLL && IsVerticalScrollActive();
            if (is_vertical_scrolling_disabled)
            {
                starting_row = line;
                fine_scroll_y = 0;
            }

            const byte table_start_x = position_x - scroll_x;
            // Which is the first tile to process
            const byte starting_column = table_start_x >> 3;
            // Which pixel starts the tile.
            const byte fine_scroll_x = table_start_x & 0b00000111;

            uint32_t tile_address = table_name;
            tile_address += starting_row * 64;   //each scanline has 32 tiles (1 tile per column) but 1 tile is 2 bytes in memory
            tile_address += starting_column * 2; // each tile is two bytes in memory

            uint16_t tile_data = m_VRam[tile_address + 1] << 8;
            tile_data += m_VRam[tile_address];

            // ---pcvhnnnnnnnnn
            const bool has_priority = tile_data & (1 << 12);
            const bool use_secondary_palette = tile_data & (1 << 11);
            const bool vertical_flip = tile_data & (1 << 10);
            const bool horizontal_flip = tile_data & (1 << 9);
            uint16_t pattern_index = tile_data & 0x1FF;

            const byte offset = (vertical_flip ? 7 - fine_scroll_y : fine_scroll_y) << 2;
            pattern_index = pattern_index << 5;
            pattern_index += offset * 4;

            // each pattern is composed by 4 bitplanes
            byte data_1 = m_VRam[pattern_index];
            byte data_2 = m_VRam[pattern_index + 1];
            byte data_3 = m_VRam[pattern_index + 2];
            byte data_4 = m_VRam[pattern_index + 3];

            const byte bit_index = horizontal_flip ? 7 - fine_scroll_x : fine_scroll_x;

            pixel_color = (data_1 & (1 << bit_index)) & 0x01;
            pixel_color += ((data_2 & (1 << bit_index)) & 0x01) << 1;
            pixel_color += ((data_3 & (1 << bit_index)) & 0x01) << 2;
            pixel_color += ((data_4 & (1 << bit_index)) & 0x01) << 3;


            if (use_secondary_palette)
            {
                pixel_color += SECONDARY_COLOR_PALETTE_OFFSET;
            }

            // @TODO: add depth buffer for priority?? 

        }

        const byte color = m_CRam[pixel_color];
        const RGBColor rgb_color = RGBColor::GetFromSMSColor(color);

        WriteToFramBuffer(line, position_x, rgb_color);
    }
}

void VDP::WriteToFramBuffer(uint32_t line, uint32_t pixel_in_line, RGBColor color)
{
    const uint32_t starting_index = line * VDP::MAX_WIDTH * NUM_COLOR_COMPONENTS + pixel_in_line * NUM_COLOR_COMPONENTS;

    m_frame_buffer[starting_index] = color.R;
    m_frame_buffer[starting_index + 1] = color.G;
    m_frame_buffer[starting_index + 2] = color.B;
}


void VDP::SetPal(bool is_pal)
{
    m_pal         = is_pal;
    m_format_dirt = true;
}

byte VDP::ReadDataPort()
{
    m_is_first_byte = true;

    byte data = m_read_buffer;

    switch (GetCodeRegister())
    {
    case 0: 
        m_read_buffer = m_VRam[GetAddressRegister()];
        break;
    case 1:
        m_read_buffer = m_VRam[GetAddressRegister()];
        break;
    default: 
        assert(false);
        break;
    }

    IncrementAddressRegister();

    return data;
}

void VDP::WriteDataPort(byte data)
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

void VDP::WriteControlPort(byte data)
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
    return !(m_registers[0] & (1 << 7));
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
    m_status_flags |= (1 << 5);
}

void VDP::SetSpriteOverflow()
{
    m_status_flags |= (1 << 6);
}

bool VDP::IsInterruptRequested() const
{
    return m_status_flags & (1 << 7) && m_registers[1] & (1 << 5);
}

bool VDP::IsBckgTableNameExtended() const
{
    return m_line_mode == LINE_MODE::MODE_224 || m_line_mode == LINE_MODE::MODE_240;
}

uint16_t VDP::GetBackgroundTableName() const
{
    const bool name_table_extended = IsBckgTableNameExtended();

    // MODE_224 and MODE_240 uses uses specific values based on the bits 2 and 3
    if (name_table_extended)
    {
        const byte value = (m_registers[2] & 0x0C) >> 2;

        switch (value)
        {
        case 0: return 0x0700;
        case 1: return 0x1700;
        case 2: return 0x2700;
        case 3: return 0x3700;       
        default: return 0x0700;
        }
    }
    else
    {
        const uint16_t map_address = (m_registers[2] & (name_table_extended ? 0x0C : 0x0E)) << 10;
        return map_address;
    }
}

byte VDP::GetOverscanColor() const
{
    return m_registers[7] & 0x0F;
}
