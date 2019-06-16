#include "ws_socket.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void ws_socket::Connection::Send(std::shared_ptr<Message> message, const OnError& onerror)
{
	if (send(m_fd, message->buf, message->len, 0) < 0)
	{
		onerror(-1, "the buffer can not send");
	}
}

ws_socket::TCP::TCP()
{
	//TODO:可以做点配置啥的
}

bool ws_socket::TCP::Connect(char* ip, int port)
{
	m_connection = std::make_shared<Connection>(socket(AF_INET, SOCK_STREAM, 0));
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);
	if (connect(m_connection->m_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == 0)
	{
		LoopThread::GetInstance()->Commit([&](void* p) -> void {
			char* buf = (char*)malloc(sizeof(char) * 1024);
			ssize_t len = recv(m_connection->m_fd, buf, 1024, MSG_DONTWAIT);
			if (len > 0) {
				char* b = (char*)malloc(sizeof(char) * len);
				memcpy(b, buf, len);
				std::shared_ptr<Message> message(new Message(b, len));
				if (m_on_message) {
					m_on_message(m_connection, message);
				}
			}
			free(buf);
			return;
		});
		return true;
	}
	m_on_close(m_connection, -1, "socket can not connect");
	return false;
}


ws_socket::LoopThread::LoopThread()
{
	m_thread = std::thread(std::bind(&LoopThread::Loop, this));
}


void ws_socket::LoopThread::Commit(const LoopWork&& work)
{
	m_works.push_back(std::move(work));
}

void ws_socket::LoopThread::Loop()
{
	for (;;)
	{
		std::vector<LoopWork>::iterator it;
		for (it = m_works.begin(); it != m_works.end(); it++)
		{
			(*it)(nullptr);
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
