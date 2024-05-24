#include "IODevice.h"
#include "VDP.h"

static constexpr byte STARTING_ADDRESS               = 0x40;
static constexpr byte MAX_COUNTER_READ_ADDRESS       = 0x7F;
static constexpr byte MAX_DATA_CONTROL_ADDRESS       = 0xBF;

byte IODevice::Read(byte address)
{
    if (address < STARTING_ADDRESS)
    {
        // Do nothing?
        return 255;
    }
    else if (address <= MAX_COUNTER_READ_ADDRESS)
    {
        // Even address - VCounter
        // Odd address - HCounter
        if (address % 2 == 0)
        {
            return m_context.vdp->GetVCounter();
        }
        else
        {
            return m_context.vdp->GetHCounter();
        }
    }
    else if (address <= MAX_DATA_CONTROL_ADDRESS)
    {
        // Even adress = data port
        // Odd address = control port
        if (address % 2 == 0)
        {
            return m_context.vdp->ReadDataPort();
        }
        else
        {
            return m_context.vdp->ReadControlPort();
        }
    }

    return 0;
}

void IODevice::Write(byte address, byte data)
{
    if (address < STARTING_ADDRESS)
    {
        // Do nothing?
        return;
    }
    else if (address <= MAX_COUNTER_READ_ADDRESS)
    {
        // Even address - VCounter
        // Odd address - HCounter
        if (address % 2 == 0)
        {
            // SN76489 data(write)
        }
        else
        {
            // SN76489 data(write, mirror)
        }
    }
    else if (address <= MAX_DATA_CONTROL_ADDRESS)
    {
        // Even adress = data port
        // Odd address = control port
        if (address % 2 == 0)
        {
            m_context.vdp->WriteDataPort(data);
        }
        else
        {
            m_context.vdp->WriteControlPort(data);
        }
    }
}
