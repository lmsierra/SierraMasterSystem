#include "TestMM.h"
#include <assert.h>

TestMM::TestMM(Memory& owner, GameRom& game_rom) : MemoryMapping(owner, game_rom)
{
}

TestMM::~TestMM()
{
}

byte TestMM::ReadMemory(word address)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

    return m_internal_memory[address];
}

void TestMM::WriteMemory(word address, byte data)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

    m_internal_memory[address] = data;
}
