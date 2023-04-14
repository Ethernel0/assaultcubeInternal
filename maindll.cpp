#include <iostream>
#include <windows.h>
#include "memc.h"

void mainLoop(HMODULE hModule);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD callReason, LPVOID lpReserved) {
    switch (callReason) {
        case DLL_PROCESS_ATTACH:
            CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) mainLoop, hModule, 0, nullptr));
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

void mainLoop(HMODULE hModule){

    const wchar_t* moduleName = L"ac_client.exe";
    DWORD procID = memc::getProcID("ac_client.exe");
    uintptr_t moduleBase = memc::getMBA(moduleName, procID);

    uintptr_t playerLocalPtr = memc::getMLP(moduleBase + 0x183828, {0x8, 0x76c, 0x96c});
    /*  above function is equal to this
    uintptr_t ptr1 = moduleBase + 0x00183828
    uintptr_t ptr2 = *(uintptr_t*)ptr1 + 0x8;
    uintptr_t ptr3 = *(uintptr_t*)ptr2 + 0x76c;
    uintptr_t finalAddr = *(uintptr_t*)ptr3 + 0x96c;
    */

    bool bConsole = false;
    FILE *F;
    AllocConsole();
    freopen_s(&F, "CONOUT$", "w", stdout);
    std::cout << "ProcID:\t" << std::dec << procID << std::endl;
    std::cout << "MBA:\t"  << *(uintptr_t*)moduleBase << std::endl;
    std::cout << "Final address: " << std::hex << playerLocalPtr << std::endl;

    auto ammo = playerLocalPtr+0x4;

    while (true){
        Sleep(800);
        std::cout << "PLP+0x4: " << playerLocalPtr+0x4 << std::endl;
        std::cout << "ammo: " << ammo << std::endl;
        *(int*)(playerLocalPtr+0x4) = 1221;
        std::cout << "intcast: " << *(int*)(playerLocalPtr+0x4);
        if (GetAsyncKeyState(VK_END)){
        }
    }

}
