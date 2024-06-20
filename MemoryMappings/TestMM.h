#pragma once

#include "MemoryMapping.h"
  
class TestMM : public MemoryMapping
{
public:
    TestMM(Memory& owner, GameRom& game_rom);
    ~TestMM();

    byte  ReadMemory(word address)            override;
    void  WriteMemory(word address, byte data) override;
};

