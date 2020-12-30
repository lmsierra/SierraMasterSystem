#include "SegaMM.h"
#include <assert.h>
#include "Types.h"

SegaMM::SegaMM(Memory& owner, GameRom& game_rom) : MemoryMapping(owner, game_rom)
{

}

SegaMM::~SegaMM()
{

}

/*
    When reading and writing, maybe pagination is needed.
    Now it can be solved by reading the cartridge memory instead of
    copying memory into the processor one, but back in the day I assume
    it was copied.
    If it outperforms badly, change the copies for just accessing the memory instead.
*/
byte SegaMM::ReadMemory(word address)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

    return m_internal_memory[address];
}

void SegaMM::WriteMemory(word address, byte data)
{
    assert(address >= 0x0000 && address < 0x10000 && "Trying to write memory out of bounds");

    // We cannot write memory in ROM.
    if (address < 0x8000)
        return;
    else if (address < 0xc000)
    {
        // Trying to write into the ROM/RAM slot.
        // We need to check if it is used by RAM or ROM,
        // so it is ROM we can't write on it

        byte rom_ram_select = m_internal_memory[0xfffc];
        const bool used_by_rom = (rom_ram_select & (1 << 3)) == 0;
        if (used_by_rom)
            return;

        m_internal_memory[address] = data;
    }
    else
    {
        // Mirror RAM data
        if (address < 0xe000)
            m_internal_memory[address + 0x2000] = data;
        else if (address < 0xfffc)
            m_internal_memory[address - 0x2000] = data;

        if (address == 0xfffc)
        {
            if((data & (1 << 3)) != 0)
            {
                const word ram_page = (data & (1 << 2)) == 0 ? 0xc000 : 0xe000;
                memcpy(&m_internal_memory[0x8000], &m_cartridge->GetRom()[ram_page], 0x2000);
            }
        }
        else if (address == 0xfffd)
        {
            const uint8_t page = data & (m_cartridge->GetNumRomBanks() - 1);
            memcpy(&m_internal_memory[0x0400], &m_cartridge->GetRom()[page*0x4000], 0x4000 - 0x0400);
        }
        else if (address == 0xfffe)
        {
            const uint8_t page = data & (m_cartridge->GetNumRomBanks() - 1);
            memcpy(&m_internal_memory[0x4000], &m_cartridge->GetRom()[page*0x4000], 0x4000);
        }
        else if (address == 0xffff)
        {
            const bool used_by_rom = (data & (1 << 3)) == 0;
            if (used_by_rom)
            {
                const uint8_t page = data & (m_cartridge->GetNumRomBanks() - 1);
                memcpy(&m_internal_memory[0x8000], &m_cartridge->GetRom()[page*0x4000], 0x4000);
            }
        }

        m_internal_memory[address] = data;
    }
}
