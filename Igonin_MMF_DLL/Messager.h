#pragma once

#include "Message.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <cwchar>

using namespace std;

extern "C" IGONIN_DLL_API void sendCommand(int from, int to, int command, const wchar_t* str);

extern "C" IGONIN_DLL_API MessageData getServerData(int from, MessageTypes type);