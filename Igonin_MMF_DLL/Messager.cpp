#include "pch.h"
#include "Messager.h"

HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMF_Mutex"); // <-- ��������� ���������� �������

extern "C" {
    IGONIN_DLL_API void sendCommand(int selected_thread, int commandId, const wchar_t* message)
    {
        try {
            Message::send(selected_thread, commandId, message);
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] ���������� ��������� �������: " << e.what() << std::endl;
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

            Message msg = Message(-1, MT_GETDATA);
            msg.send(socket);
            msg.receive(socket);

            if (msg.header.type == MT_GETDATA) {
                
                int sessionCount = stoi(msg.data);
                return sessionCount;
            }
            return 0;
        }
        catch (const std::exception& e) {
            std::wcerr << L"[DLL] ������ ��� ��������� ���������� ������: " << e.what() << std::endl;
            return -1;
        }
    }
    
}