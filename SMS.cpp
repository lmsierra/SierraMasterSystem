#include "SMS.h"
#include "Z80.h"
#include "GameRom.h"
#include "VDP.h"
#include "SDL.h"
#include <assert.h>
#include <chrono>
#include <thread>

#include <iostream>

#include "ExternalInterface/SDLInterface.h"

// https://segaretro.org/Sega_Master_System/Technical_specifications
const SystemInfo NTSCSystemInfo = SystemInfo(53693175, 262, 59.922743f, 896040);
const SystemInfo PALSystemInfo  = SystemInfo(53203424, 313, 49.701459f, 1070460);

SystemInfo::SystemInfo(uint32_t _master_clock_cycles, uint32_t _lines_per_frame, float _fps, uint32_t _max_cycles_per_frame) :
    master_clock_cycles(_master_clock_cycles), lines_per_frame(_lines_per_frame), fps(_fps), max_machine_cycles_per_frame(_max_cycles_per_frame)
{
    assert(fps > 0 && "Invalid FPS provided");
    assert(lines_per_frame && "Invalid number of line per frame provided");
    cycles_per_line = static_cast<uint32_t>(floor(clock_speed / lines_per_frame / fps));
    frame_target_time = 1000.0 / fps; // 1 second / fps
    max_clock_cycles_per_frame = max_machine_cycles_per_frame / fps;
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

    std::chrono::time_point<std::chrono::steady_clock> last_frame_time;

    while (true)
    {
        const std::chrono::time_point<std::chrono::steady_clock> current_frame = std::chrono::steady_clock::now();

        const auto delta_time = std::chrono::duration<double, std::milli>(current_frame - last_frame_time);
        last_frame_time = current_frame;

        const double time_diff = m_system_info.frame_target_time - delta_time.count();

        if (m_game_rom && m_game_rom->IsValid())
        {
            Tick();
            
            // Render results
            m_ext_interface->RenderFrame(m_vdp->GetFrameBuffer());
        }

        if (time_diff > 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(time_diff));
        }
    }

    m_ext_interface->Quit();
}

void SMS::Tick()
{
    uint32_t total_cycles = 0;
    bool vblank = false;

    while (!vblank)
    {
        const uint32_t cycles = m_cpu->Tick();

        // Z80 runs at 1/3 the speed of the machine clock
        const uint32_t machine_cycles = cycles * 3;
        
        // VDP runs at half the speed of the machine clock
        const uint32_t vpd_cycles = machine_cycles / 2;
        
        vblank = m_vdp->Tick(vpd_cycles);

        total_cycles += machine_cycles;

        // if we are above the maximum master cycles per frame, force the vblank (just in case).
        if (total_cycles >= m_system_info.max_clock_cycles_per_frame)
        {
            vblank = true;
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

        m_vdp->SetVideoSystemInfo(m_system_info.lines_per_frame, m_system_info.cycles_per_line);

        m_cpu->LoadGame(*m_game_rom);

        return true;
    }
    return false;
}

bool SMS::IsNTSC(const GameRom& game_rom)
{
    // @TODO: Implement me. Not sure how to detect this.
    //        Maybe compare to a database?
    return false;
}

