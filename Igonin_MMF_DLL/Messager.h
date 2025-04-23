#pragma once

#ifdef IGONIN_DLL_EXPORTS
#define IGONIN_DLL_API __declspec(dllexport)
#else
#define IGONIN_DLL_API __declspec(dllimport)
#endif

#include "asio.h"
#include <windows.h>
#include <string>
#include <iostream>

using namespace std;

struct header {
    int addr;
    int command;
    int size;
};


extern "C" IGONIN_DLL_API void mapSend(int addr, int command, const wchar_t* str);

IGONIN_DLL_API std::wstring mapReceive(header& h);