#pragma once

#ifdef IGONIN_DLL_EXPORTS
#define IGONIN_DLL_API __declspec(dllexport)
#else
#define IGONIN_DLL_API __declspec(dllimport)
#endif

#include <windows.h>

struct header {
    int addr;
    int size;
};

extern "C" {
    IGONIN_DLL_API HANDLE mapSend(int addr, const wchar_t* str);
    IGONIN_DLL_API bool mapReceive(header& h, wchar_t* buffer, int bufferSize);
}