#include "SMS.h"
#include "Z80.h"
#include "GameRom.h"

SMS::SMS() : m_cpu (new Z80()), m_game_rom (nullptr)
{
	m_cpu = new Z80();
}

SMS::~SMS()
{
	delete m_cpu;
	delete m_game_rom;
}

void SMS::Tick()
{
	if (m_game_rom && m_game_rom->IsValid())
	{
		uint32_t cycles = m_cpu->Tick();
	}
}

bool SMS::LoadGame(const char* path)
{
	// TODO: Reuse m_game_rom.

	delete m_game_rom;
	m_game_rom = new GameRom(path);

	if (m_game_rom->IsValid())
	{
		m_cpu->LoadGame(*m_game_rom);
		return true;
	}
	return false;
}
