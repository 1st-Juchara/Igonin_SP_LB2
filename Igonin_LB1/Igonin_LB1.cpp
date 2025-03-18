
#include "SysProg.h"
using namespace std;
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

//HANDLE hEvents[100];
//CRITICAL_SECTION cs;

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
	string data;
	Message() = default;
	Message(MessageTypes messageType, const string& data = "")
		:data(data)
	{
		header = { messageType,  int(data.length()) };
	}
};

struct header
{
	int addr;
	int size;
};

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

	void addMessage(Message& m)
	{
		EnterCriticalSection(&cs);
		messages.push(m);
		SetEvent(hEvent);
		LeaveCriticalSection(&cs);
	}

	bool getMessage(Message& m)
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

	void addMessage(MessageTypes messageType, const string& data = "")
	{
		Message m(messageType, data);
		addMessage(m);
	}
};

DWORD WINAPI MyThread(LPVOID lpParameter)
{
	auto session = static_cast<Session*>(lpParameter);
	SafeWrite("session", session->sessionID, "created");
	while (true)
	{
		Message m;
		if (session->getMessage(m))
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
				SafeWrite("session", session->sessionID, "data", m.data);
				Sleep(500 * session->sessionID);
				break;
			}
			}
		}
	}
	return 0;
}

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
	CloseHandle(hFile);
}

void start()
{
    vector<Session*> sessions;
    vector<HANDLE> threads;
    InitializeCriticalSection(&cs);
    int i = 1;

    HANDLE hStartEvent = CreateEvent(NULL, FALSE, FALSE, "StartEvent");
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, "StopEvent");
    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, "ConfirmEvent");
	//sam pisal
	HANDLE hCloseEvent = CreateEvent(NULL, FALSE, FALSE, "CloseEvent");
    //
	HANDLE hControlEvents[3] = { hStartEvent, hStopEvent, hCloseEvent };
	//sam pisal
    while (i)
    {
        int n = WaitForMultipleObjects(3, hControlEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        switch (n)
        {
        case 0:
			sessions.push_back(new Session(i++));
			threads.push_back(CreateThread(NULL, 0, MyThread, (LPVOID)sessions.back(), 0, NULL));
            SetEvent(hConfirmEvent);
            break;
        case 1:
            sessions.back()->addMessage(MT_CLOSE);
			WaitForSingleObject(threads.back(), INFINITE);
			sessions.pop_back();
			threads.pop_back();
			i--;
            SetEvent(hConfirmEvent);
			if (i == 1)
				return;
            break;
		case 2:
			return;
			break;
        }
    }
	//
    SetEvent(hConfirmEvent);
    DeleteCriticalSection(&cs);
}

int main()
{
    start();
    return 0;
}
