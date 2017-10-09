/*
    Initial author: Convery (tcn@ayria.se)
    Started: 9-10-2017
    License: MIT
    Notes:
        Quick bypass for the STMSIG anti-tampering system.
*/

#include "../Stdinclude.h"

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

    // Copy the unknown ID.
    DRM.Unknown = *(uint32_t *)Bufferpointer;
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

    // Read the GUID.
    Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
    Start.GUID = { Filepointer, Buffersize }; Filepointer += Buffersize;

    // Read the Split GUID.
    Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
    Start.SplitGUID = { Filepointer, Buffersize }; Filepointer += Buffersize;

    // Read the instance ID.
    Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
    Start.Instance = { Filepointer, Buffersize }; Filepointer += Buffersize;

    // Read the modulename.
    Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
    Start.Modulename = { Filepointer, Buffersize }; Filepointer += Buffersize;

    // Read the full path.
    Buffersize = *(uint32_t *)Filepointer; Filepointer += sizeof(uint32_t);
    Start.Fullpath = { Filepointer, Buffersize }; Filepointer += Buffersize;

    // Read the unknown var even though it's likely just an EOF token.
    Start.Unknown = *(uint32_t *)Filepointer;
}
