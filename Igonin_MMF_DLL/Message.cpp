#include "pch.h"
#include "Message.h"

void MessageServer::send(tcp::socket& s, int to, int type, const wstring& data)
{
	MessageServer m(to, type, data);
	m.send(s);
}

MessageServer MessageServer::send(int to, int type, const wstring& data)
{
	boost::asio::io_context io;
	tcp::socket s(io);
	tcp::resolver r(io);
	boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));

	MessageServer m(to, type, data);
	m.send(s);
	return m;
}

