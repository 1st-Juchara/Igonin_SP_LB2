#include "SysProg.h"

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

void launchClient(string path)
{
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	CreateProcess(NULL, path.data(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

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

	void AddMessage(int to, MessageTypes messageType, const wstring& data = L"")
	{
		Message m(to, messageType, data);
		AddMessage(m);
	}
};

vector<M_Session*> sessions;
mutex sessionsMutex;
int max_ID = 0;

DWORD WINAPI MyThread(LPVOID lpParameter)
{
	auto session = static_cast<M_Session*>(lpParameter);
	SafeWrite("session", session->sessionID, "created");
	while (true)
	{
		Message m;
		if (session->GetMessage(m))
		{
			switch (m.header.type)
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
				if (!saveWstringToFile(format("message_{}.txt", session->sessionID), m.data))
					SafeWriteW(L"session", session->sessionID, L"data error");
				break;
			}
			}
		}
	}
	return 0;
}


void processClient(tcp::socket s)
{
	try
	{
		Message m;
		int code = m.receive(s);
		switch (code)
		{
			case MT_INIT:
			{
				unique_lock<mutex> lock(sessionsMutex);
				sessions.push_back(new M_Session(++max_ID));
				thread t(MyThread, (LPVOID)sessions.back());
				t.detach();
				break;

				//unique_lock<mutex> lock(sessionsMutex);
				//auto session = make_shared<Session>(max_ID++,m.data);
				//thread t(MyThread, (LPVOID)sessions.back());
				//t.detach();
				//sessions[session->id] = session;
				//Message::send(s, session->id, MT_INIT);
				//break;
			}
			case MT_CLOSE:
			{
				if (max_ID > 0)
				{
					sessions.back()->AddMessage(m.header.to, MT_CLOSE);

					sessions.pop_back();
					max_ID--;
				}
				break;
			}
			case MT_GETDATA:
			{
				unique_lock<mutex> lock(sessionsMutex);

				MessageHeader responseHeader;
				responseHeader.type = MT_GETDATA;
				responseHeader.to = -1;
				responseHeader.size = sizeof(int);

				sendData(s, &responseHeader, sizeof(responseHeader));
				sendData(s, &max_ID, sizeof(int));
				break;
			}
			case MT_DATA:
			{
				if (m.header.to == 0)
				{
					SafeWriteW(m.data);
				}
				else if (m.header.to == 1)
				{
					for (auto s : sessions)
						s->AddMessage(m.header.to, MT_DATA, m.data);
				}
				else
				{
					sessions[m.header.to - 2]->AddMessage(m.header.to, MT_DATA, m.data);
				}
				break;
			}
			default:
			{
				//cout << "CLIENT IS EXIT (-_o)" << endl;
				//auto iSessionTo = sessions.find(m.header.to);
				//if (iSessionTo == sessions.end())
				//{
				//	iSessionTo->second->add(m);
				//}
				//else if (m.header.to == 0)
				//{
				//	for (auto& [id, session] : sessions)
				//	{
				//		if (id != m.header.to)
				//			session->add(m);
				//	}
				//}
				//Message::send(s, m.header.to, MT_CONFIRM);
				
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		std::wcerr << "Exception: " << e.what() << endl;
	}
}



int main()
{
	std::locale::global(std::locale("rus_rus.866"));
	wcin.imbue(std::locale());
	wcout.imbue(std::locale());

	try
	{
		int port = 12345;
		boost::asio::io_context io;
		tcp::acceptor a(io, tcp::endpoint(tcp::v4(), port));
		for (int i = 0; i < 3; i++)
			launchClient("Igonin_Form.exe");

		while (true)
		{
			std::thread(processClient, a.accept()).detach();
		}
	}
	catch (std::exception& e)
	{
		std::wcerr << "Exception: " << e.what() << endl;
	}

	return 0;
}
