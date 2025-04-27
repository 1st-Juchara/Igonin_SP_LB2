#include "pch.h"
#include "Messager.h"

HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMF_Mutex"); // <-- добавляем глобальный мьютекс

extern "C" {
    IGONIN_DLL_API void sendCommand(int selected_thread, int commandId, const wchar_t* message)
    {
        try {
            boost::asio::io_context io;
            tcp::socket socket(io);
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            boost::asio::connect(socket, endpoints);

            MessageHeader header;
            header.type = commandId;
            header.size = message ? static_cast<int>(wcslen(message) * sizeof(wchar_t)) : 0;
            header.to = selected_thread;

            sendData(socket, &header, sizeof(header));
            if (header.size > 0)
                sendData(socket, message, header.size);
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] Невозможно отправить команду: " << e.what() << std::endl;
        }
    }

    IGONIN_DLL_API int getSessionCount()
    {
        try {
            boost::asio::io_context io;
            tcp::socket socket(io);
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            boost::asio::connect(socket, endpoints);

            MessageHeader header;
            header.type = MT_GETDATA;
            header.size = 0;
            header.to = -1;

            sendData(socket, &header, sizeof(header));

            MessageHeader responseHeader;
            boost::asio::read(socket, boost::asio::buffer(&responseHeader, sizeof(responseHeader)));

            if (responseHeader.type == MT_GETDATA && responseHeader.size == sizeof(int)) {
                int sessionCount;
                boost::asio::read(socket, boost::asio::buffer(&sessionCount, sizeof(int)));
                return sessionCount;
            }
            return 0;
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] Ошибка при получении количества сессий: " << e.what() << std::endl;
            return 0;
        }
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
