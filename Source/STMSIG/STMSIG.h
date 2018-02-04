/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 9-10-2017
    License: MIT
    Notes:
        Quick bypass for the STMSIG anti-tampering system.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace STMSIGBypass
{
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
        uint32_t Activeprocess;
        std::string Startupmodule;
        std::string Startevent;
        std::string Termevent;
    };
    struct SteamStart
    {
        std::string GUID;
        std::string SplitGUID;
        std::string Instance;
        std::string Modulename;
        std::string Fullpath;
        uint32_t Unknown;

        /*
            Raw STARTUPINFOW struct and all the strings in it.
            Not going to bother with it.
        */
    };

    void InitializeIPC(SteamIPC &IPC);
    void InitializeDRM(SteamDRM &DRM, char *Bufferpointer);
    void InitializeSteamstart(SteamStart &Start, const char *Filepath);

    void HookModulehandle();
}
