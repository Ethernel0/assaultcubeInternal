//
// Created by Igni on 26/03/2023.
//

#ifndef ACINTERNAL_MEMC_H
#define ACINTERNAL_MEMC_H

#pragma once

#include <windows.h>
#include <iostream>
#include <vector>

namespace memc{
    DWORD       getProcID(LPCSTR ProcessName);
    uintptr_t   getMBA(const wchar_t *moduleName, DWORD procID);
    uintptr_t   getMLP(uintptr_t pointer, std::vector<unsigned int> offsets);
    bool        writeMem(HANDLE hProc, uintptr_t address, const void* buffer, size_t size);
    void        memPatch(BYTE* dst, BYTE* src, unsigned int size);
}


#endif //ACINTERNAL_MEMC_H
