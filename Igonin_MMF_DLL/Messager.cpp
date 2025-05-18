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

    IGONIN_DLL_API MessageData getServerData(int from, MessageTypes type)
    {
        MessageData msgData = { };
        try {
            boost::asio::io_context io;
            tcp::socket socket(io);
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            boost::asio::connect(socket, endpoints);

            Message msg = Message(from, -1, type);
            msg.send(socket);
            msg.receive(socket);

            MessageData msgData = { };

            msgData = { msg.header, {0} };
            if (msg.header.size > 0)
            {
                wcsncpy_s(msgData.data, msg.data.c_str(), 249);
            }
            return msgData;
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] Ошибка при получении количества сессий: " << e.what() << std::endl;
            msgData = { {-2, -2, -2}, L"ERROR"};
        }
        return msgData;
    }
    
}