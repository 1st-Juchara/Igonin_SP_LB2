#pragma once
#include "iostream"
using namespace std;



struct header
{
    int addr;
    int size;
};

HANDLE mapsend(int addr, const char* str)
{
    header h = { addr, strlen(str) + 1 };
    HANDLE hFile = CreateFile("filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);
    HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, h.size + sizeof(header), "MyMap");
    char* buff = (char*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, h.size + sizeof(header));

    memcpy(buff, &h, sizeof(header));
    memcpy(buff + sizeof(header), str, h.size);


    UnmapViewOfFile(buff);
    return hFileMap;
    //    CloseHandle(hFileMap);
    CloseHandle(hFile);
}

std::string mapreceive(header& h)
{
    HANDLE hFile = CreateFile("filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);

    HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(header), "MyMap");
    LPVOID buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(header));
    h = *((header*)buff);
    UnmapViewOfFile(buff);
    CloseHandle(hFileMap);

    int n = h.size + sizeof(header);
    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, n, "MyMap");
    buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, n);
    //   cout << GetLastErrorString();
    string s((char*)buff + sizeof(header), h.size);

    UnmapViewOfFile(buff);
    CloseHandle(hFileMap);
    return s;
}

extern "C" _declspec(dllexport) void MapSend(int addr, const char* str) {
    mapsend(addr)
}

