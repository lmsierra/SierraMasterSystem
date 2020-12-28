#include "Memory.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "GameRom.h"
#include "MemoryMappings/MemoryMapping.h"
#include "MemoryMappings/SegaMM.h"
#include "MemoryMappings/ROMOnlyMM.h"

Memory::Memory() : m_memory_mapping(nullptr)
{
    m_memory = (byte*) calloc(0x10000 - 1, sizeof(byte));
}

Memory::~Memory()
{
    free(m_memory);
}

byte Memory::ReadMemory(const word& address) const
{
    if (address < 0x0400)
        return m_memory[address];

    assert(m_memory != nullptr && "m_memory must not be null");
    return m_memory_mapping->ReadMemory(address);
}

void Memory::WriteMemory(const word& address, byte data)
{
    assert(m_memory != nullptr && "m_memory must not be null");
    m_memory_mapping->WriteMemory(address, data);
}

void Memory::LoadRom(GameRom& game_rom)
{
    Reset();

    const long rom_size = game_rom.GetSize();
    memcpy(m_memory, game_rom.GetRom(), rom_size < 0xc000 ? rom_size : 0xc000);

    switch (game_rom.GetMemoryType())
    {
    case MemoryType::Sega:        m_memory_mapping = new SegaMM(*this, game_rom);    break;
    case MemoryType::ROMOnly:     m_memory_mapping = new ROMOnlyMM(*this, game_rom); break;
    case MemoryType::Codemasters: m_memory_mapping = nullptr;                        break;
    default: break;
    }
}

void Memory::Reset()
{
    free(m_memory);
    m_memory = (byte*)calloc(0x10000 - 1, sizeof(byte));
}
