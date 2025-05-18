#pragma once
#include "asio.h"

#ifdef IGONIN_DLL_EXPORTS
#define IGONIN_DLL_API __declspec(dllexport)
#else
#define IGONIN_DLL_API __declspec(dllimport)
#endif

IGONIN_DLL_API enum MessageTypes
{
	MT_INIT,
	MT_CLOSE,
	MT_GETDATA,
	MT_DATA,
	MT_NODATA,
	MT_CONFIRM,
	MT_EXIT
};

IGONIN_DLL_API struct MessageHeader
{
	int from;
	int to;
	int type;
	int size;
};

IGONIN_DLL_API struct MessageData {
	MessageHeader header;
	wchar_t data[250];
};

IGONIN_DLL_API class Message
{
public:

	MessageHeader header = { 0 };
	wstring data;

	Message() {}
	Message(int from, int to, int type = MT_DATA, const wstring& data = L"")
	{
		this->data = data;
		header = { from, to, type, int(data.length() * sizeof(wchar_t)) };
	}

	void send(tcp::socket& s)
	{
		sendData(s, &header);
		if (header.size)
		{
			sendData(s, data.c_str(), header.size);
		}
	}

	//можно было бы сделать receive по подобию send статическим, но в примере он
	//здесь, так что я не менял
	int receive(tcp::socket& s)
	{
		receiveData(s, &header);
		if (header.size)
		{
			data.resize(header.size / sizeof(wchar_t));
			receiveData(s, data.data(), header.size);
		}
		return header.type;
	}

	IGONIN_DLL_API static void send(tcp::socket& s, int from, int to, int type = MT_DATA, const wstring& data = L"");

	IGONIN_DLL_API static Message send(int from, int to, int type = MT_DATA, const wstring& data = L"");
};
