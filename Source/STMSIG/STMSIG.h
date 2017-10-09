/*
    Initial author: Convery (tcn@ayria.se)
    Started: 9-10-2017
    License: MIT
    Notes:
        Quick bypass for the STMSIG anti-tampering system.
*/

#pragma once
#include "../Stdinclude.h"

struct SteamIPC
{
    HANDLE Consumesemaphore;
    HANDLE Producesemaphore;
    HANDLE Sharedfilehandle;
    void *Sharedfilemapping;
};
struct SteamDRM
{
    uint32_t ProcessID;
    uint32_t Unknown;
    std::string Startupmodule;
    std::string Startevent;
    std::string Termevent;
};

void InitializeIPC(SteamIPC &IPC);
