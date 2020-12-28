#pragma once

#include "MemoryMapping.h"

class ROMOnlyMM : public MemoryMapping
{
public:
	ROMOnlyMM(Memory& owner, GameRom& game_rom);
	~ROMOnlyMM();

	byte  ReadMemory  (word address)            override;
	void  WriteMemory (word address, byte data) override;
};


