#include "SMS.h"
#include "Z80.h"
#include "GameRom.h"
#include "VDP.h"
#include "SDL.h"
#include <assert.h>

#include "ExternalInterface/SDLInterface.h"

// https://segaretro.org/Sega_Master_System/Technical_specifications
const SystemInfo NTSCSystemInfo = SystemInfo(53693175, 262, 59.922743f, 896040);
const SystemInfo PALSystemInfo  = SystemInfo(53203424, 313, 49.701459f, 1070460);

SystemInfo::SystemInfo(uint32_t _master_clock_cycles, uint32_t _lines_per_frame, float _fps, uint32_t _max_cycles_per_frame) :
	master_clock_cycles(_master_clock_cycles), lines_per_frame(_lines_per_frame), fps(_fps), max_cycles_per_frame(_max_cycles_per_frame)
{
	assert(fps > 0 && "Invalid FPS provided");
	assert(lines_per_frame && "Invalid number of line per frame provided");
	cycles_per_line = static_cast<uint32_t>(ceil(clock_speed / lines_per_frame / fps));
}

SMS::SMS()
{
	m_cpu			= new Z80();
	m_vdp			= new VDP(*m_cpu);
	m_game_rom		= nullptr;

	m_ext_interface = new SDLInterface();
}

SMS::~SMS()
{
	delete m_cpu;
	delete m_vdp;
	delete m_game_rom;
}

void SMS::Launch(const char* path)
{
	assert(m_ext_interface != nullptr);
	assert(m_cpu != nullptr);
	assert(m_vdp != nullptr);

	const bool is_game_loaded = LoadGame(path);
	if (!is_game_loaded)
		return;

	m_ext_interface->InitWindow(VDP::MAX_WIDTH * 2, VDP::MAX_HEIGHT * 2 , VDP::MAX_WIDTH, VDP::MAX_HEIGHT);

	while (true)
	{
		Tick();
	}

	m_ext_interface->Quit();
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

			m_ext_interface->RenderFrame(m_vdp->GetFrameBuffer());
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

