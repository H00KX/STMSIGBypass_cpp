/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 09-01-2018
    License: MIT
    Notes:
        Provides the entrypoint for Windows and Nix.
*/

#include "Stdinclude.hpp"

namespace STMSIGBypass
{
    void Initialize()
    {
        SteamIPC IPC{};
        SteamDRM DRM{};
        SteamStart Start{};

        // Initialize the IPC and wait for data.
        InitializeIPC(IPC);
        WaitForSingleObject(IPC.Consumesemaphore, 1500);

        // Initialize the DRM struct from the data.
        InitializeDRM(DRM, (char *)IPC.Sharedfilemapping);

        // Read the startup file, even though steam just copies this to a "./STF" temp-file.
        InitializeSteamstart(Start, DRM.Startupmodule.c_str());

        // Remove the startupfile.
        std::remove(DRM.Startupmodule.c_str());

        // Acknowledge that the game has started.
        auto Event = OpenEventA(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, DRM.Startevent.c_str());
        SetEvent(Event);
        CloseHandle(Event);

        // Notify the game that we are done.
        ReleaseSemaphore(IPC.Producesemaphore, 1, NULL);

        // Clean up the IPC.
        UnmapViewOfFile(IPC.Sharedfilemapping);
        CloseHandle(IPC.Sharedfilehandle);
        CloseHandle(IPC.Consumesemaphore);
        CloseHandle(IPC.Producesemaphore);

        // Hook the modulehandle to return the wrong modulename.
        HookModulehandle();
    }
}

// Callbacks from the bootstrapper (Bootstrapmodule_cpp).
extern "C" EXPORT_ATTR void onInitializationStart(bool Reserved)
{
    // Initialize in a different thread.
    std::thread(STMSIGBypass::Initialize).detach();
}

#if defined _WIN32
BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    switch (nReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Opt-out of further thread notifications.
            DisableThreadLibraryCalls(hDllHandle);

            // Clear the previous sessions logfile.
            Clearlog();
        }
    }

    return TRUE;
}
#else
__attribute__((constructor)) void DllMain()
{
    // Clear the previous sessions logfile.
    Clearlog();
}
#endif
