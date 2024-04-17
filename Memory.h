#pragma once

#include "Types.h"
#include <string>

/*
Memory mapping:

    0x0000
            1KB ROM 0 Bank
    0x0400
            15KB ROM Page 0
    0x4000
            16KB ROM Page 1
    0x8000
            16KB ROM Page 2 or Cartridge RAM
    0xc000
            8KB on-board RAM
    0xe000
            Mirror of 8KB on-board RAM
    0xfffc
            ROM/RAM select.

            bit 7 : ? ?
                6 : ? ?
                5 : ? ?
                4 : ? ?
                3 : 0 = Page 2 mapped as ROM as from register $FFFF
                    1 = Page 2 mapped as on-board cartridge RAM
                2 : 0 = Use first page of cartridge RAM
                    1 = Use second page of cartridge RAM
                1 : ? ?
                0 : ? ?

    0xfffd
            Page 0 ROM
    0xfffe
            Page 1 ROM
    0xffff
            Page 2 ROM
    0x10000
*/

class GameRom;
class MemoryMapping;
class Memory
{
public:
    Memory();
    ~Memory();

    byte ReadMemory  (const word& address) const;
    void WriteMemory (const word& address, byte value);
    void LoadRom	 (GameRom& game_rom);
    void Reset       ();
    
    const byte* GetMemory() const { return m_memory; }
    byte*		GetMemory()       { return m_memory; }

private:
    byte* m_memory; // Map of the whole memory.
    MemoryMapping* m_memory_mapping;
};
