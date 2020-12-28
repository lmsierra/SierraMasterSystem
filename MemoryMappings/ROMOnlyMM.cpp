#include "ROMOnlyMM.h"
#include <assert.h>

ROMOnlyMM::ROMOnlyMM(Memory& owner, GameRom& game_rom) : MemoryMapping(owner, game_rom)
{
}

ROMOnlyMM::~ROMOnlyMM()
{
}

byte ROMOnlyMM::ReadMemory(word address)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

	return m_internal_memory[address];
}

void ROMOnlyMM::WriteMemory(word address, byte data)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

    // We cannot write memory in ROM.
    if (address < 0xc000)
        return;

    // Mirror RAM data
    if (address < 0xe000)
        m_internal_memory[address + 0x2000] = data;
    else if (address < 0xfffc)
        m_internal_memory[address - 0x2000] = data;

    // Finally write in memory.
    m_internal_memory[address] = data;
}
