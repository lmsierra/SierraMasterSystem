#pragma once

#include <string>
#include "Types.h"

/*
    SMS games contains a 16 bytes header with the following structure:

    Value		      Size			   Usage:
    --------------    --------------   --------------------
    TMR SEGA          (8 bytes)        - ASCII text of TMR Sega required for the BIOS to validate the game.
    Reserved space    (2 bytes)        - Unused
    Checksum          (2 bytes)        - ROM's checksum. Seems japanese cardtriges doesn't contain a valid one ??.
    Product code      (2.5 bytes)      - Product code. 2 digits + 4 bits
    Version code      (0.5 bytes)      - Version code.
    Region Code       (0.5 bytes)      - Region code:
                                            3 = SMS Japan
                                            4 = SMS Export
                                            5 = GG Japan
                                            6 = GG Export
                                            7 = GG International
    ROM size		  (0.5 bytes)      - Size of the ROM.
                                            a = 8KB 	Unused
                                            b = 16KB	Unused
                                            c = 32KB	 
                                            d = 48KB	Unused, buggy
                                            e = 64KB	Rarely used
                                            f = 128KB	 
                                            0 = 256KB	 
                                            1 = 512KB	Rarely used
                                            2 = 1024KB  Unused, buggy
    
    The header can be located at three different addresses: 0x1ff0, 0x3ff0 or 0x7ff0
*/

enum class MemoryType : byte
{
    None = 0,
    Sega,
    ROMOnly,
    Codemasters
};

class GameRom
{
    enum class RegionCode : byte
    {
        SMS_Japan = 3,
        SMS_Export,
        GG_Japan,
        GG_Export,
        GG_International
    };

public:
    GameRom(const std::string& path);
    ~GameRom();

    long        GetSize        () const;
    RegionCode  GetRegionCode  () const;
    uint8_t     GetNumRomBanks () const;
    MemoryType  GetMemoryType  () const { return m_type;  }
    const byte* GetRom         () const { return m_rom;   }
    byte*       GetRom         ()       { return m_rom;   }
    bool        IsValid        () const { return m_valid; }

protected:
    void Load(const std::string& path);
    void ReadHeader();

private:
    byte*      m_rom;
    byte       m_region_and_size;
    MemoryType m_type;
    bool	   m_valid;
    uint8_t    m_num_banks;

protected:
    static bool    FindHeaderAddress (byte* rom, word& out_address);
    static bool    IsHeaderValid     (byte* rom, word header_address);
    static long    GetSizeInBytes    (byte b);
};
