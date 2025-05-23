#include "pch.h"
#include "Message.h"

IGONIN_DLL_API void Message::send(tcp::socket& s, int from, int to, int type, const wstring& data)
{
	Message m(from, to, type, data);
	m.send(s);
}

//с предустановленным сокетом
IGONIN_DLL_API Message Message::send(int from, int to, int type, const wstring& data)
{
	boost::asio::io_context io;
	tcp::socket s(io);
	tcp::resolver r(io);
	boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));

	Message m(from, to, type, data);
	m.send(s);
	return m;
}

