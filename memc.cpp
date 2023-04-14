#include "memc.h"
#include <psapi.h>
#include <TlHelp32.h>

DWORD memc::getProcID(LPCTSTR ProcessName)
{
    PROCESSENTRY32 pt;
    HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pt.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hsnap, &pt)) { // must call this first
        do {
            if (!lstrcmpi(pt.szExeFile, ProcessName)) {
                CloseHandle(hsnap);
                return pt.th32ProcessID;
            }
        } while (Process32Next(hsnap, &pt));
    }
    CloseHandle(hsnap); // close handle on failure
    return 0;
}

uintptr_t memc::getMBA(const wchar_t* moduleName, DWORD processID)
{
    uintptr_t moduleBaseAddress = 0;

    // Open process
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (processHandle == nullptr) {
        return moduleBaseAddress;
    }

    // Enumerate modules to find the module with the specified name
    HMODULE moduleHandles[1024];
    DWORD numModules = 0;
    if (EnumProcessModules(processHandle, moduleHandles, sizeof(moduleHandles), &numModules)) {
        numModules /= sizeof(HMODULE);
        for (DWORD i = 0; i < numModules; i++) {
            wchar_t moduleNameBuffer[MAX_PATH];
            if (GetModuleFileNameExW(processHandle, moduleHandles[i], moduleNameBuffer, MAX_PATH)) {
                wchar_t* moduleNamePtr = wcsrchr(moduleNameBuffer, L'\\');
                if (moduleNamePtr != nullptr) {
                    moduleNamePtr++;  // move past the backslash
                    if (_wcsicmp(moduleNamePtr, moduleName) == 0) {
                        MODULEINFO moduleInfo;
                        if (GetModuleInformation(processHandle, moduleHandles[i], &moduleInfo, sizeof(moduleInfo))) {
                            moduleBaseAddress = (uintptr_t)moduleInfo.lpBaseOfDll;
                            break;
                        }
                    }
                }
            }
        }
    }

    CloseHandle(processHandle);
    return moduleBaseAddress;
}

uintptr_t memc::getMLP(uintptr_t pointer, std::vector<unsigned int> offsets) {
    uintptr_t addr = pointer;
    for (unsigned int i=0; i < offsets.size(); i++){
        addr = *(uintptr_t*)addr;
        addr += offsets[i];

    }
    return addr;
}

bool memc::writeMem(HANDLE hProc, uintptr_t address, const void *buffer, size_t size) {
    LPVOID remoteAddress = VirtualAllocEx(hProc, NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (remoteAddress == NULL){
        std::cerr << "Error: unable to allocate memory in target process" << std::endl;
        return false;
    }

    BOOL success = WriteProcessMemory(hProc, remoteAddress, buffer, size, NULL);
    if (!success){
        std::cerr << "Error: unable to write memory to target process" << std::endl;
    }

    success = WriteProcessMemory(hProc, (LPVOID)address, &remoteAddress, sizeof(LPVOID), NULL);
    if (!success){
        std::cerr << "Error: unable to allocate memory in target process" << std::endl;
        VirtualFreeEx(hProc, remoteAddress, 0, MEM_RELEASE);
        return false;
    }

    VirtualFreeEx(hProc, remoteAddress, 0, MEM_RELEASE);
    return true;
}

void memc::memPatch(BYTE *dst, BYTE *src, unsigned int size) {
    DWORD oldProtect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}