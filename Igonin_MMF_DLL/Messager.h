#pragma once

#include "Message.h"
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

extern "C" IGONIN_DLL_API int __stdcall getSessionCount();

IGONIN_DLL_API std::wstring mapReceive(header& h);