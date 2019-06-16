#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <vector>

namespace ws_socket 
{
	static void Error(int error, const std::string& reason)
	{
		std::cerr << "erro code:" << error << "->" << reason << std::endl;
		return;
	}

	class LoopThread;

	class Message
	{
	public:
		Message(char* buf, ssize_t len) : buf(buf), len(len){};
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

	typedef std::function<void(std::shared_ptr<Connection>, std::shared_ptr<Message>)> OnMessage;
	typedef std::function<void(std::shared_ptr<Connection>, const int status, const std::string& reason)> OnClose;

	class TCP
	{
	public:
		TCP();
		TCP(const TCP&) = delete;
		TCP& operator=(const TCP&) = delete;
		void set_on_message(const OnMessage& onmessage) { m_on_message = onmessage; };
		void set_on_close(const OnClose& onclose) { m_on_close = onclose; };
		bool Connect(char* ip, int port);
		std::shared_ptr<Connection> connection() { return m_connection; };

	private:
		std::shared_ptr<Connection> m_connection;
		OnMessage m_on_message;
		OnClose m_on_close;
	};


	static std::atomic<LoopThread*> sLoopThread(nullptr);

	class LoopThread {
	public:
		typedef std::function<void(void* args)> LoopWork;

	private:
		LoopThread();
	public:
		LoopThread(const LoopThread&) = delete;
		LoopThread& operator= (const LoopThread&) = delete;

		static LoopThread* GetInstance()
		{
			if (sLoopThread == nullptr)
			{
				sLoopThread = new LoopThread();
			}
			return sLoopThread;
		}

		void Commit(const LoopWork&& work);

	private:
		void Loop();

	private:
		std::thread m_thread;
		std::vector<LoopWork> m_works;
	};
}