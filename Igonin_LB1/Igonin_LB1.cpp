/*
#include "SysProg.h"

using namespace std;

bool saveWstringToFile(const string& filePath, const wstring& content) {
	// Открываем файл для записи в широком символьном режиме
	std::wofstream outFile(filePath, std::ios::out); // std::ios::out - режим записи

	// Устанавливаем локаль для корректной работы с Unicode (опционально)
	outFile.imbue(std::locale("")); // Системная локаль

	// Проверяем, открылся ли файл
	if (!outFile.is_open()) {
		return false;
	}

	// Записываем содержимое
	outFile << content;

	// Закрываем файл (не обязательно, т.к. деструктор сделает это автоматически)
	outFile.close();

	// Проверяем, не было ли ошибок
	return !outFile.fail();
}


enum MessageTypes
{
	MT_M_CLOSE,
	MT_M_DATA,
};

struct MessageHeader
{
	int messageType;
	int size;
};

struct Message
{
	MessageHeader header = { 0 };
	wstring data;
	Message() = default;
	Message(MessageTypes messageType, const wstring& data = L"")
		:data(data)
	{
		header = { messageType,  int(data.length()) };
	}
};

class M_Session
{
	queue<Message> messages;
	CRITICAL_SECTION cs;
	HANDLE hEvent;
public:
	int sessionID;

	M_Session(int sessionID) :sessionID(sessionID)
	{
		InitializeCriticalSection(&cs);
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~M_Session()
	{
		DeleteCriticalSection(&cs);
		CloseHandle(hEvent);
	}

	void AddMessage(Message& m)
	{
		EnterCriticalSection(&cs);
		messages.push(m);
		SetEvent(hEvent);
		LeaveCriticalSection(&cs);
	}

	bool GetMessage(Message& m)
	{
		bool res = false;
		WaitForSingleObject(hEvent, INFINITE);
		EnterCriticalSection(&cs);
		if (!messages.empty())
		{
			res = true;
			m = messages.front();
			messages.pop();
		}
		if (messages.empty())
		{
			ResetEvent(hEvent);
		}
		LeaveCriticalSection(&cs);
		return res;
	}

	void AddMessage(MessageTypes messageType, const wstring& data = L"")
	{
		Message m(messageType, data);
		AddMessage(m);
	}
};

DWORD WINAPI MyThread(LPVOID lpParameter)
{
	auto session = static_cast<M_Session*>(lpParameter);
	SafeWrite("session", session->sessionID, "created");
	while (true)
	{
		Message m;
		if (session->GetMessage(m))
		{
			switch (m.header.messageType)
			{
			case MT_M_CLOSE:
			{
				SafeWrite("session", session->sessionID, "closed");
				delete session;
				return 0;
			}
			case MT_M_DATA:
			{
				SafeWriteW(L"session", session->sessionID, L"data", m.data);
				if (!saveWstringToFile(format("message_{}.txt", session->sessionID), m.data))
					SafeWriteW(L"session", session->sessionID, L"data error");
				break;
			}
			}
		}
	}
	return 0;
}

void start()
{
    vector<M_Session*> sessions;
    InitializeCriticalSection(&cs);
    int i = 1;

    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, "ConfirmEvent");

	HANDLE hMessageEvent = CreateEvent(NULL, FALSE, FALSE, "MessageEvent");

	while (i)
	{
		WaitForSingleObject(hMessageEvent, INFINITE);
		header h = { 0, 0, 0 };
		wstring msg = mapReceive(h);
        switch (h.command)
        {
        case 0:
		{
			sessions.push_back(new M_Session(i++));
			thread t(MyThread, (LPVOID)sessions.back());
			t.detach();
			SetEvent(hConfirmEvent);
			break;
		}
        case 1:
		{
			sessions.back()->AddMessage(MT_M_CLOSE);
			
			sessions.pop_back();
			i--;
			SetEvent(hConfirmEvent);
			if (i == 1)
				return;
			break;
		}
		case 2:
		{
			if (h.addr == 0)
			{
				SafeWriteW(msg);
			}
			else if (h.addr == 1)
			{
				for (auto s : sessions)
					s->AddMessage(MT_M_DATA, msg);
			}
			else
			{
				sessions[h.addr - 2]->AddMessage(MT_M_DATA, msg);
			}
			SetEvent(hConfirmEvent);
			break;

		}
		case 3:
		{
			return;
			break;
		}
		default:
			SafeWriteW(L"SC ERROR");
        }
    }
	//
    SetEvent(hConfirmEvent);
    DeleteCriticalSection(&cs);
}

int main()
{
	setlocale(LC_CTYPE, "Russian");
    start();
    return 0;
}
*/