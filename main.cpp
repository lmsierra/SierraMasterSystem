#include "Memory.h"
#include "GameRom.h"
#include "Z80.h"
#include "SMS.h"
#include <iostream>

int main()
{
/*
	Memory* mem = new Memory();

	GameRom game = GameRom("Roms/Taz-Mania.sms");
	if(game.IsValid())
		mem->LoadRom(game);

	Z80 cpu;
*/

	SMS sms;
	sms.LoadGame("Roms/Taz-Mania.sms");
	// sms.LoadGame("Roms/Alex Kidd in Miracle World.sms");
	while (true)
	{
		sms.Tick();
	}

	return 0;
}