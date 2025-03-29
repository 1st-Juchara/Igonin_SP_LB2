#pragma once

#include <windows.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include "Messager.h"

using namespace std;

inline void DoWrite()
{
	std::cout << std::endl;
}

template <class T, typename... Args> inline void DoWrite(T& value, Args... args)
{
	std::cout << value << " ";
	DoWrite(args...);
}

static CRITICAL_SECTION cs;
static bool initCS = true;
template <typename... Args> inline void SafeWrite(Args... args)
{
	if (initCS)
	{
		InitializeCriticalSection(&cs);
		initCS = false;
	}
	EnterCriticalSection(&cs);
	DoWrite(args...);
	LeaveCriticalSection(&cs);
}

inline void DoWriteW()
{
	std::wcout << std::endl;
}

template <class T, typename... Args> inline void DoWriteW(T& value, Args... args)
{
	std::wcout << value << " ";
	DoWriteW(args...);
}

template <typename... Args> inline void SafeWriteW(Args... args)
{
	if (initCS)
	{
		InitializeCriticalSection(&cs);
		initCS = false;
	}
	EnterCriticalSection(&cs);
	DoWriteW(args...);
	LeaveCriticalSection(&cs);
}