#pragma once

#include "Types.h"

class VDP;
struct IODeviceContext
{
    IODeviceContext() {}
    IODeviceContext(VDP* _vdp) : vdp(_vdp) {}

    VDP* vdp = nullptr;
};

class IODevice
{
public:
    IODevice() {}

public: // inline
    inline void SetContext(const IODeviceContext& context) { m_context = context; }

public:
    byte Read  (byte address);
    void Write (byte address, byte data);

private:
    IODeviceContext m_context;
};
