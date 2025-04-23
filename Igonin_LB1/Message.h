//#pragma once
//#include "Messager.h"
//
//enum MessageTypes2
//{
//	MT_INIT,
//	MT_EXIT,
//	MT_GETDATA,
//	MT_DATA,
//	MT_NODATA,
//	MT_CONFIRM
//};
//
////enum MessageRecipients
////{
////	MR_BROKER = 10,
////	MR_ALL = 50,
////	MR_USER = 100
////};
//
//struct MessageHeader2
//{
//	int to;
//	int type;
//	int size;
//};
//
//
//class MessageServer
//{
//public:
//	MessageHeader2 header = { 0 };
//	wstring data;
//
//	MessageServer() {}
//	MessageServer(int to, int type = MT_DATA, const wstring& data = L"")
//	{
//		this->data = data;
//		header = { to, type, int(data.length() * sizeof(wchar_t)) };
//	}
//
//	void send(tcp::socket& s)
//	{
//		sendData(s, &header);
//		if (header.size)
//		{
//			sendData(s, data.c_str(), header.size);
//		}
//	}
//
//	int receive(tcp::socket& s)
//	{
//		receiveData(s, &header);
//		if (header.size)
//		{
//			data.resize(header.size / sizeof(wchar_t));
//			receiveData(s, data.data(), header.size);
//		}
//		return header.type;
//	}
//
//	static void send(tcp::socket& s, int to, int type = MT_DATA, const wstring& data = L"");
//
//	static MessageServer send(int to, int type = MT_DATA, const wstring& data = L"");
//};
