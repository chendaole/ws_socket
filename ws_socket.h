#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <vector>
#include "thread_pool.h"

namespace wssocket 
{
	static void Error(int error, const std::string& reason)
	{
		std::cerr << "erro code:" << error << "->" << reason << std::endl;
		return;
	}

	class Message
	{
	public:
		Message(void* buf, ssize_t len) : buf(buf), len(len){};
		~Message() { free(buf); };
		void* buf;
		ssize_t len;
	};

	class Connection : std::enable_shared_from_this<Connection> {
	public:
		typedef std::function<void(int error, const std::string& reason)> OnError;

		Connection() {};
		Connection(int fd) : m_fd(fd) {};
		void Send(std::shared_ptr<Message> message, const OnError& onError = std::bind(Error, std::placeholders::_1, std::placeholders::_2));

	public:
		int m_fd;
	};

	class Filter {
	public:
		typedef std::function<void(std::shared_ptr<Message>)> OnSuccess;
		typedef std::function<void(int error, const std::string& reason)> OnFailed;
		virtual void In(std::shared_ptr<Message>) = 0;
		void set_on_success(const OnSuccess& onsuccess) { m_on_success = onsuccess; };
		void set_on_failed(const OnFailed& onfailed) { m_on_failed = onfailed; };

	protected:
		OnSuccess m_on_success;
		OnFailed m_on_failed;
	};

	class TCP
	{
	public:
		typedef std::function<void(std::shared_ptr<Connection>, std::shared_ptr<Message>)> OnMessage;
		typedef std::function<void(std::shared_ptr<Connection>, const int status, const std::string& reason)> OnClose;
		TCP();
		TCP(const TCP&) = delete;
		TCP& operator=(const TCP&) = delete;
		~TCP();
		void set_on_message(const OnMessage& onmessage) { m_on_message = onmessage; };
		void set_on_close(const OnClose& onclose) { m_on_close = onclose; };
		void set_filter(Filter* filter) { m_filter = filter; };
		bool Connect(char* ip, int port);
		std::shared_ptr<Connection> connection() { return m_connection; };

	private:
		std::shared_ptr<Connection> m_connection;
		OnMessage m_on_message;
		OnClose m_on_close;
		ThreadPool* m_pool = nullptr;
		Filter* m_filter = nullptr;
	};
}