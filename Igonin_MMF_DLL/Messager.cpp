#include "pch.h"
#include "Messager.h"

HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMF_Mutex"); // <-- добавляем глобальный мьютекс

extern "C" {
    IGONIN_DLL_API void sendCommand(int from, int to, int commandId, const wchar_t* message)
    {
        try {
            Message::send(from, to, commandId, message);
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] Невозможно отправить команду: " << e.what() << std::endl;
        }
    }

    IGONIN_DLL_API int getSessionCount(int from)
    {
        try {
            boost::asio::io_context io;
            tcp::socket socket(io);
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            boost::asio::connect(socket, endpoints);

            Message msg = Message(from, -1, MT_GETDATA);
            msg.send(socket);
            msg.receive(socket);

            if (msg.header.type == MT_GETDATA) {
                
                int sessionCount = stoi(msg.data);
                return sessionCount;
            }
            return 0;
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] Ошибка при получении количества сессий: " << e.what() << std::endl;
            return -1;
        }
    }
    
}