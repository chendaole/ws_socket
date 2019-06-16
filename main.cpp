// CMakeProject2.cpp: 定义应用程序的入口点。
//
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "ws_socket.h"

int main(int argc, char** argv)
{
	ws_socket::TCP tcpsocket;
	tcpsocket.set_on_message([&](std::shared_ptr<ws_socket::Connection> con, std::shared_ptr<ws_socket::Message> msg) -> void {
		std::cout << msg->buf << std::endl;
		return;
	});
	tcpsocket.set_on_close([&](std::shared_ptr<ws_socket::Connection> contection, const int status, const std::string& reason) -> void {
		std::cout << "error code:" << status << ", reason:" << reason << std::endl;
		return;
	});

	tcpsocket.Connect("127.0.0.1", 34);

	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	return 0;
}