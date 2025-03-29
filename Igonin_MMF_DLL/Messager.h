#pragma once

#ifdef IGONIN_DLL_EXPORTS
#define IGONIN_DLL_API __declspec(dllexport)
#else
#define IGONIN_DLL_API __declspec(dllimport)
#endif

#include <windows.h>
#include <string>
#include <iostream>

struct header {
    int addr;
    int size;
};

extern "C" IGONIN_DLL_API void mapSend(int addr, const wchar_t* str);

IGONIN_DLL_API std::wstring mapReceive(header& h);