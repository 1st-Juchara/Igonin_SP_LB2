#pragma once
#include <conio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;
using namespace std;

template <class T>
inline void sendData(tcp::socket& s, T* data, size_t n = 0)
{
	if (!n)
		n = sizeof(T);
	boost::system::error_code error;
	boost::asio::write(s, boost::asio::buffer(data, n), error);
	if (error)
		throw boost::system::system_error(error);
}

template <class T>
inline void receiveData(tcp::socket& s, T* data, size_t n = 0)
{
	if (!n)
		n = sizeof(T);
	boost::system::error_code error;
	boost::asio::read(s, boost::asio::buffer(data, n), error); 
	if (error)
		throw boost::system::system_error(error);
}
