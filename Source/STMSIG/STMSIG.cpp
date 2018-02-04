/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 9-10-2017
    License: MIT
    Notes:
        Quick bypass for the STMSIG anti-tampering system.
*/

#include "../Stdinclude.hpp"

namespace STMSIGBypass
{
    // Setup the IPC.
    void InitializeIPC(SteamIPC &IPC)
    {
        SECURITY_DESCRIPTOR pSecurityDescriptor;
        InitializeSecurityDescriptor(&pSecurityDescriptor, 1);
        SetSecurityDescriptorDacl(&pSecurityDescriptor, 1, 0, 0);

        SECURITY_ATTRIBUTES SemaphoreAttributes;
        SemaphoreAttributes.nLength = 12;
        SemaphoreAttributes.bInheritHandle = FALSE;
        SemaphoreAttributes.lpSecurityDescriptor = &pSecurityDescriptor;

        IPC.Consumesemaphore = CreateSemaphoreA(&SemaphoreAttributes, 0, 512, "STEAM_DIPC_CONSUME");
        IPC.Producesemaphore = CreateSemaphoreA(&SemaphoreAttributes, 1, 512, "SREAM_DIPC_PRODUCE");    // Intentional typo.
        IPC.Sharedfilehandle = CreateFileMappingA(INVALID_HANDLE_VALUE, &SemaphoreAttributes, 4u, 0, 0x1000u, "STEAM_DRM_IPC");
        IPC.Sharedfilemapping = MapViewOfFile(IPC.Sharedfilehandle, 0xF001Fu, 0, 0, 0);
    }

    // Copy data from the shared buffer.
    void InitializeDRM(SteamDRM &DRM, char *Bufferpointer)
    {
        // Copy the process ID.
        DRM.ProcessID = *(uint32_t *)Bufferpointer;
        Bufferpointer += sizeof(uint32_t);

        // Copy the process status.
        DRM.Activeprocess = *(uint32_t *)Bufferpointer;
        Bufferpointer += sizeof(uint32_t);

        // Copy the startup module-name.
        DRM.Startupmodule = Bufferpointer;
        Bufferpointer += DRM.Startupmodule.length() + 1;

        // Copy the startup event-name.
        DRM.Startevent = Bufferpointer;
        Bufferpointer += DRM.Startevent.length() + 1;

        // Copy the termination event-name.
        DRM.Termevent = Bufferpointer;
    }

    // Read the steamstart file from disk. (Because why use the IPC for that..)
    void InitializeSteamstart(SteamStart &Start, const char *Filepath)
    {
        uint32_t Buffersize;
        auto Filebuffer = Readfile(Filepath);
        auto Filepointer = Filebuffer.data();
        if (Filebuffer.size() == 0) return;

        // Read the GUID.
        Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
        Start.GUID = std::string(Filepointer, Buffersize); Filepointer += Buffersize;

        // Read the Split GUID.
        Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
        Start.SplitGUID = std::string(Filepointer, Buffersize); Filepointer += Buffersize;

        // Read the instance ID.
        Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
        Start.Instance = std::string(Filepointer, Buffersize); Filepointer += Buffersize;

        // Read the modulename.
        Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
        Start.Modulename = std::string(Filepointer, Buffersize); Filepointer += Buffersize;

        // Read the full path.
        Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
        Start.Fullpath = std::string(Filepointer, Buffersize); Filepointer += Buffersize;

        // Read the unknown var even though it's likely just an EOF token.
        Start.Unknown = *(uint32_t *)Filepointer;
    }

    // Replace the modulehandle with the steamclients.
    constexpr const char *Clientlibrary = sizeof(void *) == sizeof(uint64_t) ? "steamclient64.dll" : "steamclient.dll";
    Hooking::StomphookEx<decltype(GetModuleHandleExA)> Modulehook;
    BOOL __stdcall _GetModuleHandleExA(DWORD dwFlags, LPCTSTR lpModuleName, HMODULE *phModule)
    {
        BOOL Result;
        HMODULE Module;
        static char Filename[512];
        static std::mutex Threadguard;

        Threadguard.lock();
        {
            // Get the real module.
            Modulehook.Removehook();
            Result = GetModuleHandleExA(dwFlags, lpModuleName, &Module);
            Modulehook.Reinstall();

            // Check against steam_api and platformwrapper.
            GetModuleFileNameA(Module, Filename, 512);
            if (std::strstr(Filename, "steam_api") || std::strstr(Filename, "Platformwrapper"))
                Module = GetModuleHandleA(Clientlibrary);
        }
        Threadguard.unlock();

        *phModule = Module;
        return Result;
    }
    void HookModulehandle()
    {
        // Get the hook address.
        auto Address = GetProcAddress(LoadLibraryA("Kernel32.dll"), "GetModuleHandleExA");
        Modulehook.Installhook(Address, &_GetModuleHandleExA);
    }
}
