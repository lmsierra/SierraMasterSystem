#include "Memory.h"
#include "GameRom.h"
#include "Z80.h"
#include "SMS.h"
#include <iostream>

int main()
{
    SMS sms;
    
    sms.Launch("Roms/Taz-Mania.sms");
    // sms.Launch("Roms/zexall_sdsc.sms");
    
    return 0;
}
