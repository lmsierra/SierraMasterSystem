#pragma once

#include "Types.h"
#include "Memory.h"
#include "GameRom.h"

class MemoryMapping
{
public:
	MemoryMapping(Memory& owner, GameRom& game_rom)
		: m_internal_memory (owner.GetMemory())
		, m_cartridge       (&game_rom)
	{}
	virtual byte  ReadMemory  (word address) = 0;
	virtual void  WriteMemory (word address, byte data) = 0;

protected:
	byte* m_internal_memory;
	GameRom* m_cartridge;
};
