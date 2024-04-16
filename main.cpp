#include "Memory.h"
#include "GameRom.h"
#include "Z80.h"
#include "SMS.h"
#include <iostream>

int main()
{
	SMS sms;
	sms.Launch("Roms/Taz-Mania.sms");
	// sms.LoadGame("Roms/Taz-Mania.sms");
	

	return 0;
}