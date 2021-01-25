#include "SMS.h"
#include "Z80.h"
#include "GameRom.h"
#include "VDP.h"

// https://segaretro.org/Sega_Master_System/Technical_specifications
constexpr SystemInfo  NTSCSystemInfo = SystemInfo(53693175, 262, 59.922743f, 896040);
constexpr SystemInfo  PALSystemInfo  = SystemInfo(53203424, 313, 49.701459f, 1070460);

SMS::SMS()
{
	m_cpu      = new Z80();
	m_vdp      = new VDP(m_cpu);
	m_game_rom = nullptr;
}

SMS::~SMS()
{
	delete m_cpu;
	delete m_vdp;
	delete m_game_rom;
}

void SMS::Tick()
{
	if (m_game_rom && m_game_rom->IsValid())
	{
		uint32_t total_cycles = 0;
		bool vblank = false;

		while (!vblank)
		{
			uint32_t cycles = m_cpu->Tick();
			vblank = m_vdp->Tick(cycles);

			total_cycles += cycles;
			
			// if we are above the maximum master cycles per frame, force the vblank (just in case).
			if (total_cycles >= m_system_info.max_cycles_per_frame)
			{
				vblank = true;
			}
		}
	}
}

bool SMS::LoadGame(const char* path)
{
	// TODO: Reuse m_game_rom.

	delete m_game_rom;
	m_game_rom = new GameRom(path);

	if (m_game_rom->IsValid())
	{
		m_system_info = IsNTSC(*m_game_rom) ? NTSCSystemInfo : PALSystemInfo; 

		m_cpu->LoadGame(*m_game_rom);

		return true;
	}
	return false;
}

bool SMS::IsNTSC(const GameRom& game_rom)
{
	// @TODO: Implement me. Not sure how to detect this.
	//        Maybe compare to a database?
	return true;
}

