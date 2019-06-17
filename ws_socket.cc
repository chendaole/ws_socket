#include "ws_socket.h"
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void wssocket::Connection::Send(std::shared_ptr<Message> message, const OnError& onerror)
{
	if (send(m_fd, message->buf, message->len, 0) < 0)
	{
		onerror(-1, "the buffer can not send");
	}
}

wssocket::TCP::TCP()
	:m_pool(new ThreadPool(4))
{
	//TODO:可以做点配置啥的
	signal(SIGPIPE, SIG_IGN);
}

wssocket::TCP::~TCP()
{
	delete m_pool;
}

bool wssocket::TCP::Connect(char* ip, int port)
{
	m_connection = std::make_shared<Connection>(socket(AF_INET, SOCK_STREAM, 0));
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);
	if (connect(m_connection->m_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == 0)
	{
		std::future<void > ret = m_pool->Commit([&]() -> void {

			while (true)
			{
				void* buf = malloc(sizeof(char) * 1024);
				//非堵塞获取
				ssize_t len = recv(m_connection->m_fd, buf, 1024, 0);
				if (len > 0) {
					void* b = malloc(sizeof(char) * len);
					memcpy(b, buf, len);
					std::shared_ptr<Message> message(new Message(b, len));
					if (m_on_message) {
						if (m_filter) {
							m_filter->set_on_success([&](std::shared_ptr<Message> newmessage) -> void {
								m_on_message(m_connection, newmessage);
								return;
								});
							m_filter->set_on_failed([&](int error, const std::string& reason) { return; });
							m_filter->In(message);
						}
						else
						{
							m_on_message(m_connection, message);
						}
					}
				}
				else
				{
					if (m_on_close)
					{
						m_on_close(m_connection, -1, "socket close");
					}
				}
				free(buf);
			}

			return;
		});
		ret.get();
		return true;
	}
	m_on_close(m_connection, -1, "socket can not connect");
	return false;
}

