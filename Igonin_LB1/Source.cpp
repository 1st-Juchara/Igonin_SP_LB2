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
	queue<Message> messagesToSend;
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

	void AddMessage(int from, int to, MessageTypes messageType, const wstring& data = L"")
	{
		Message m(from, to, messageType, data);
		AddMessage(m);
	}

	void AddMessageToSend(Message& m)
	{
		EnterCriticalSection(&cs);
		messagesToSend.push(m);
		LeaveCriticalSection(&cs);
	}

	void AddMessageToSend(int from, int to, MessageTypes messageType, const wstring& data = L"")
	{
		Message m(from, to, messageType, data);
		AddMessageToSend(m);
	}

	void SendMessage(tcp::socket &s, int from, int to, MessageTypes messageType, const wstring& data = L"")
	{
		EnterCriticalSection(&cs);
		Message m(from, to, messageType, data);
		m.send(s);
		LeaveCriticalSection(&cs);
	}
};

vector<M_Session*> sessions;
vector<int> ClientsID;
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
		sessionsMutex.lock();
		int code = m.receive(s);
		//SafeWriteW(L"Начало процесса");
		//SafeWriteW(L"From: ", m.header.from, L"To: ", m.header.to, L"Type: ", m.header.type);
		switch (code)
		{
			case MT_INIT:
			{

				sessions.push_back(new M_Session(++max_ID));
				ClientsID.push_back(max_ID);
				SafeWriteW("Session ", max_ID, " init");

				thread t(MyThread, (LPVOID)sessions.back());
				t.detach();
				sessions.back()->SendMessage(s, m.header.from, max_ID, MT_INIT);
				break;
			}
			case MT_CLOSE:
			{
				if (sessions.size())
				{
					sessions.back()->AddMessage(m.header.from, m.header.to, MT_CLOSE);
					sessions.pop_back();
					ClientsID.erase(ClientsID.begin() + m.header.from - 1);
				}
				break;
			}
			case MT_GETDATA:
			{
				if (sessions.size())
				{
					if (sessions[m.header.from - 1]->messagesToSend.empty())
					{
						wstring IDs = L"";
						for (int id : ClientsID)
						{
							IDs += to_wstring(id) + L' ';
						}
						sessions[m.header.from - 1]->SendMessage(s, m.header.from, -1, MT_GETDATA, IDs);
					}
					else
					{
						sessions[m.header.from - 1]->messagesToSend.front().send(s);
						sessions[m.header.from - 1]->messagesToSend.pop();
						SafeWriteW("Client", m.header.from, "recieve message");
					}
				}
				else
				{
					Message msg = Message(m.header.from, -1, MT_GETDATA, to_wstring(max_ID));
					msg.send(s);
				}
				break;
				
			}
			case MT_DATA:
			{
				if (m.header.to == 0)
				{
					for (auto s : sessions)
						s->AddMessageToSend(m.header.from, m.header.to, MT_DATA, m.data);
				}
				else
				{
					sessions[m.header.to - 1]->AddMessageToSend(m.header.from, m.header.to, MT_DATA, m.data);
					SafeWriteW("Message from", m.header.from, " to ", m.header.to, ": ", m.data);
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		std::wcerr << "Exception: " << e.what() << endl;
	}
	sessionsMutex.unlock();
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
