#include "pch.h"
#include "Messager.h"

HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMF_Mutex"); // <-- добавляем глобальный мьютекс

extern "C" {
    IGONIN_DLL_API HANDLE mapSend(int addr, const wchar_t* str)
    {
        WaitForSingleObject(hMutex, INFINITE); // <-- добавляем мьютекс

        header h = { addr, int(wcslen(str) * sizeof(wchar_t)) };
        //HANDLE hFile = CreateFile(L"filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);

        HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, h.size + sizeof(header), L"MyMap");
        BYTE* buff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, h.size + sizeof(header));

        memcpy(buff, &h, sizeof(header));
        memcpy(buff + sizeof(header), str, h.size);

        UnmapViewOfFile(buff);

        ReleaseMutex(hMutex);   // <-- освобождаем мьютекс
        return hFileMap;
    }

    IGONIN_DLL_API bool mapReceive(header& h, wchar_t* buffer, int bufferSize)
    {
        if (!buffer || bufferSize <= 0) return false;

        //HANDLE hFile = CreateFile(L"filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);
        //HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(header), L"MyMap");

        WaitForSingleObject(hMutex, INFINITE); // <-- добавляем мьютекс

        HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MyMap");
        //if (!hFileMap) return false;

        LPVOID buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(header));
        h = *((header*)buff);
        UnmapViewOfFile(buff);

        int n = h.size + sizeof(header);
        //hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, n, L"MyMap");
        buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, n);
        //wstring s((wchar_t*)((BYTE*)buff + sizeof(header)), h.size / 2);
        wcsncpy_s(buffer, bufferSize, (wchar_t*)((BYTE*)buff + sizeof(header)), h.size / sizeof(wchar_t));
        buffer[h.size / sizeof(wchar_t)] = L'\0';

        UnmapViewOfFile(buff);
        CloseHandle(hFileMap);
        //CloseHandle(hFile);

        ReleaseMutex(hMutex);
        return true;
    }
}