#pragma once
#include "Message.h"

class Session
{
public:
	int id;
	wstring name;
	queue<MessageServer> messages;

	mutex mx;
	Session(int id, wstring name)
		:id(id), name(name)
	{
	}

	void add(MessageServer& m)
	{
		lock_guard<mutex> lg(mx);
		messages.push(m);
	}

	void send(tcp::socket& s)
	{
		lock_guard<mutex> lg(mx);
		if (messages.empty())
		{
			MessageServer::send(s, id, MT_NODATA);
		}
		else
		{
			messages.front().send(s);
			messages.pop();
		}
	}
};
