#pragma once

class Z80;
class GameRom;
class SMS
{
public:
	SMS();
	~SMS();

public:
	void Tick();
	bool LoadGame(const char* path);

private:
	Z80* m_cpu;
	GameRom* m_game_rom;
};
