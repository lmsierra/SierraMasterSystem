#include "GameRom.h"
#include "FileUtils.h"
#include <iostream>
#include <bitset>
#include <assert.h>

GameRom::GameRom(const std::string& path) 
    : m_rom             (nullptr),
      m_region_and_size (0),
      m_type            (MemoryType::None),
      m_valid           (false)
{
    Load(path);
    ReadHeader();
}

GameRom::~GameRom()
{
    free(m_rom);
}

void GameRom::Load(const std::string& path)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file)
        return;
    
    const long file_size = FileUtils::GetFileSize(file);
    m_rom = (byte*)calloc(file_size, sizeof(byte));
    
    if (m_rom)
    {
        fread(m_rom, sizeof(byte), file_size, file);
    }

    fclose(file);
}

void GameRom::ReadHeader()
{
    if (!m_rom)
        return;
    
    word header_address = 0;
    const bool header_found = FindHeaderAddress(m_rom, header_address);
    
    if (!header_found)
        return;
    
    m_region_and_size = m_rom[header_address + 15];

    const byte size = m_region_and_size & 0x0f;
    if (size >= 0xA && size <= 0xc)
        m_type = MemoryType::ROMOnly;
    else
        m_type = MemoryType::Sega;
    
    m_num_banks = static_cast<uint8_t>(GetSize() / 0x4000);

    m_valid = true;
}

bool GameRom::FindHeaderAddress(byte* rom, word& out_address)
{
    static const word header_addresses[3] = { 0x7ff0, 0x3ff0, 0x1ff0 }; // Ordered by % of success finding them at that location
    for (const int header_address : header_addresses)
    {
        if (IsHeaderValid(rom, header_address))
        {
            out_address = header_address;
            return true;
        }
    }
    return false;
}

bool GameRom::IsHeaderValid(byte* rom, word header_address)
{
    if (!rom)
        return false;

    static const std::string tmr_sega = "TMR SEGA";
    for (int i = 0; i < tmr_sega.length(); i++)
    {
        if (tmr_sega[i] != rom[header_address + i * sizeof(byte)])
        {
            return false;
        }
    }

    return true;
}

uint8_t GameRom::GetNumRomBanks() const
{
    return m_num_banks;
}

long GameRom::GetSize() const
{
    return GetSizeInBytes(m_region_and_size & 0x0f);
}

GameRom::RegionCode GameRom::GetRegionCode() const
{
    return static_cast<GameRom::RegionCode>((m_region_and_size & 0xf0) >> 4);
}

long GameRom::GetSizeInBytes(byte b)
{
    switch (b)
    {
    case 0x0: return  256 * 1024;
    case 0x1: return  512 * 1024;
    case 0x2: return 1024 * 1024;
    case 0xA: return    8 * 1024;
    case 0xB: return   16 * 1024;
    case 0xC: return   32 * 1024;
    case 0xD: return   48 * 1024;
    case 0xE: return   64 * 1024;
    case 0xF: return  128 * 1024;
    default:  return 0;
    }
}
