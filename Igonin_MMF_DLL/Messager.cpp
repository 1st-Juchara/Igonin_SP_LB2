#include "pch.h"
#include "Messager.h"

HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMF_Mutex"); // <-- добавляем глобальный мьютекс

extern "C" {
    IGONIN_DLL_API void mapSend(int addr, const wchar_t* str)
    {
        WaitForSingleObject(hMutex, INFINITE); 
        header h = { addr, int(wcslen(str) * sizeof(wchar_t)) };
        HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, h.size + sizeof(header), L"MyMap");
        BYTE* buff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, h.size + sizeof(header));

        memcpy(buff, &h, sizeof(header));
        memcpy(buff + sizeof(header), str, h.size);

        UnmapViewOfFile(buff);

        ReleaseMutex(hMutex);
    }

    
}

IGONIN_DLL_API std::wstring mapReceive(header& h)
{
    WaitForSingleObject(hMutex, INFINITE);

    HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MyMap");

    LPVOID buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(header));
    h = *((header*)buff);
    UnmapViewOfFile(buff);

    int n = h.size + sizeof(header);
    buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, n);
    std::wstring s((wchar_t*)((BYTE*)buff + sizeof(header)), h.size / 2);

    UnmapViewOfFile(buff);
    CloseHandle(hFileMap);

    ReleaseMutex(hMutex);
    return s;
}