#pragma once

#include "MemoryMapping.h"

class SegaMM : public MemoryMapping
{
public:
    SegaMM(Memory& owner, GameRom& game_rom);
    ~SegaMM();

    byte  ReadMemory  (word address)            override;
    void  WriteMemory (word address, byte data) override;
};

