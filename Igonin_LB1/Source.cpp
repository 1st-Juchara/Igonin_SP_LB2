#include "SysProg.h"

void launchClient(string path)
{
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	CreateProcess(NULL, path.data(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}
map<int, shared_ptr<Session>> sessions;

int max_ID = 0;

void processClient(tcp::socket s)
{
	try
	{
		MessageServer m;
		int code = m.receive(s);
		cout << m.header.to << ": " << m.header.type << ": " << code << endl;
		switch (code)
		{
			case MT_INIT:
			{
				cout << "AB INIT IS SeND (0_o)" << endl;
				auto session = make_shared<Session>(max_ID++,m.data);
				sessions[session->id] = session;
				MessageServer::send(s, session->id, MT_INIT);
				break;
			}
			case MT_EXIT:
			{
				sessions.erase(m.header.to);
				MessageServer::send(s, m.header.to, MT_CONFIRM);
				break;
			}
			case MT_GETDATA:
			{
				auto iSession = sessions.find(m.header.to);
				if (iSession != sessions.end())
				{
					iSession->second->send(s);
				}
				break;
			}
			default:
			{
				cout << "ABUBA CONfIRM IS SeNDDDOO (-_o)" << endl;
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
				MessageServer::send(s, m.header.to, MT_CONFIRM);
				
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
