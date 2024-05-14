#pragma once

#include "Types.h"
#include <math.h>

struct SystemInfo
{
    SystemInfo() : master_clock_cycles(0), fps(0.0f), max_machine_cycles_per_frame(0), lines_per_frame(0), cycles_per_line(0), frame_target_time(0), max_clock_cycles_per_frame(0) {}
    SystemInfo(uint32_t _master_clock_cycles, uint32_t _lines_per_frame, float _fps, uint32_t _max_cycles_per_frame);

    uint32_t clock_speed                  = 3580000; // 3.58 MHz
    uint32_t master_clock_cycles          = 0;       // Console master clock rate.
    float    fps                          = 0;       // master frames per second. (master_clock_cycles / max_cycles_per_frame)
    uint32_t max_machine_cycles_per_frame = 0;       // master clock cycles per frame. (master_clock_cycles / fps)
    uint32_t lines_per_frame              = 0;       // Number of scanlines in a frame.
    // https://www.smspower.org/forums/13530-VDPClockSpeed
    uint32_t cycles_per_line              = 0;       // Number of cycles needed to process a scanline.
    double   frame_target_time            = 0;       // Time a frame is expected to take
    double   max_clock_cycles_per_frame   = 0;       // max_machine_cycles_per_frame / fps
};

class Z80;
class GameRom;
class VDP;
class ExternalInterface;
class SMS
{
public:
    SMS();
    ~SMS();

public:
    /* @TODO: decouple launch from loading a game */ 
    void Launch(const char* path);
    bool LoadGame(const char* path);

private:
    void Tick();
    static bool IsNTSC(const GameRom& game_rom);

private:
    Z80*		       m_cpu;
    GameRom*	       m_game_rom;
    VDP*		       m_vdp;
    SystemInfo	       m_system_info;
    ExternalInterface* m_ext_interface;

    double LastFrameTimestamp = 0.0;
};
