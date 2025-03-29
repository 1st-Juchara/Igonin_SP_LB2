
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
	MT_CLOSE,
	MT_DATA,
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

//struct header
//{
//	int addr;
//	int size;
//};

class Session
{
	queue<Message> messages;
	CRITICAL_SECTION cs;
	HANDLE hEvent;
public:
	int sessionID;

	Session(int sessionID) :sessionID(sessionID)
	{
		InitializeCriticalSection(&cs);
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~Session()
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
	auto session = static_cast<Session*>(lpParameter);
	SafeWrite("session", session->sessionID, "created");
	while (true)
	{
		Message m;
		if (session->GetMessage(m))
		{
			switch (m.header.messageType)
			{
			case MT_CLOSE:
			{
				SafeWrite("session", session->sessionID, "closed");
				delete session;
				return 0;
			}
			case MT_DATA:
			{
				SafeWriteW(L"session", session->sessionID, L"data", m.data);
				//Sleep(500 * session->sessionID);
				//Write to file
				if (!saveWstringToFile("message.txt", m.data))
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
    vector<Session*> sessions;
    //vector<HANDLE> threads;
    InitializeCriticalSection(&cs);
    int i = 1;

    HANDLE hStartEvent = CreateEvent(NULL, FALSE, FALSE, "StartEvent");
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, "StopEvent");
    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, "ConfirmEvent");
	//sam pisal
	HANDLE hCloseEvent = CreateEvent(NULL, FALSE, FALSE, "CloseEvent");
	HANDLE hMessageEvent = CreateEvent(NULL, FALSE, FALSE, "MessageEvent");
    //
	HANDLE hControlEvents[4] = { hStartEvent, hStopEvent, hCloseEvent, hMessageEvent };
	//sam pisal
    while (i)
    {
        int n = WaitForMultipleObjects(4, hControlEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        switch (n)
        {
        case 0:
		{
			sessions.push_back(new Session(i++));
			//threads.push_back(CreateThread(NULL, 0, MyThread, (LPVOID)sessions.back(), 0, NULL));
			thread t(MyThread, (LPVOID)sessions.back());
			t.detach();
			SafeWrite("ABUBA");
			SetEvent(hConfirmEvent);
			break;
		}
        case 1:
		{
			sessions.back()->AddMessage(MT_CLOSE);
			//WaitForSingleObject(threads.back(), INFINITE);
			//shared_lock<shared_mutex> ul(evMutex); // !!!NEW!!! !!!NEW!!! !!!NEW!!! !!!NEW!!! !!!NEW!!! !!!NEW!!! !!!NEW!!! !!!NEW!!!
			
			sessions.pop_back();
			//threads.pop_back();
			i--;
			SetEvent(hConfirmEvent);
			if (i == 1)
				return;
			break;
		}
		case 2:
			return;
			break;
		case 3:
			header h = {0, 0};
			wstring msg = mapReceive(h);
			sessions[h.addr]->AddMessage(MT_DATA, msg);
			//SafeWriteW(L"comment: ", msg);
			SetEvent(hConfirmEvent);
			break;
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
