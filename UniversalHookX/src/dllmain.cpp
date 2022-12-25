#include <Windows.h>
#include <iostream>
#include <thread>

#include "console/console.hpp"

#include "hooks/hooks.hpp"
#include "utils/utils.hpp"

#include "dependencies/minhook/MinHook.h"

DWORD WINAPI OnProcessAttach(LPVOID lpParam);
DWORD WINAPI OnProcessDetach(LPVOID lpParam);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);

        U::SetRenderingBackend(OPENGL);

        HANDLE hHandle = CreateThread(NULL, 0, OnProcessAttach, hinstDLL, 0, NULL);
        if (hHandle != NULL) {
            CloseHandle(hHandle);
        }
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        OnProcessDetach(NULL);
    }

    return TRUE;
}

void iBunnyHop( ) {
start:
    CONST int scrolldown = -120;
    CONST int scrollup = 120;
        while (true) {
        if (GetAsyncKeyState(VK_SPACE)) {
                while (true) {
                    Sleep(20);                                         // play around with this for other games, 20 is best for cs 1.6
                    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, scrollup, 0); // make sure you have mwheelup binded to +jump in game
                    if (GetKeyState(VK_SPACE)) {
                        goto start;
                    }
                }
            }
            if (GetKeyState(VK_END))
                break;
            else if ((GetKeyState(VK_SPACE) & 0x8000)) {
                continue;
            } 
        }
}


DWORD WINAPI OnProcessAttach(LPVOID lpParam) {
    MH_Initialize( );

    Console::Alloc( );
    LOG("[+] Rendering backend: %s\n", U::RenderingBackendToStr( ));
    if (U::GetRenderingBackend( ) == NONE) {
        LOG("[!] Looks like you forgot to set a backend. Will unload after pressing enter...");
        std::cin.get( );

        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(lpParam), 0);
        return 0;
    }

    H::Init( );
    iBunnyHop( );
    return 0;
}

DWORD WINAPI OnProcessDetach(LPVOID lpParam) {
    // If the process quits leave memory management to the OS.
    // H::bShuttingDown == true means we pressed end an must free them by ourself.
    if (H::bShuttingDown) {
        H::Free( );
        MH_Uninitialize( );
    }

    Console::Free( );

    return 0;
}
