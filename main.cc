// CMakeProject2.cpp: 定义应用程序的入口点。
//
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <string.h>
#include "ws_socket.h"

class IM4399Filter : public wssocket::Filter {
public:
	IM4399Filter(ssize_t headersize, ssize_t footsize)
		: m_header_size(headersize),
		m_foot_size(footsize),
		m_buffer(0)
	{
		m_buffer = malloc(256 * 1024);
	};

	~IM4399Filter() { free(m_buffer); };

	void In(std::shared_ptr<wssocket::Message> message)
	{
		memcpy(m_buffer + m_cursor, message->buf, message->len);
		m_cursor += message->len;


		for (;;)
		{
			if (m_cursor < m_header_size)
			{
				this->m_on_failed(-1, "less buffer");
				return;
			}
			else
			{
				uint32_t block = *((uint32_t*)((char*)m_buffer + m_header_size - sizeof(uint32_t)));
				uint32_t datasize = htonl(block);
				ssize_t fullsize = m_header_size + datasize + m_foot_size;
				if (m_cursor < fullsize)
				{
					this->m_on_failed(-1, "less buffer");
					return;
				}
				else if (m_cursor > fullsize)
				{
					void* buf = malloc(fullsize);
					memcpy(buf, m_buffer, fullsize);
					m_cursor = m_cursor - fullsize;
					memmove(m_buffer, (char*)m_buffer + fullsize, m_cursor);
					std::shared_ptr<wssocket::Message> newmessage = std::make_shared<wssocket::Message>(buf, fullsize);
					m_on_success(newmessage);
					continue;
				}
				else
				{
					void* buf = malloc(fullsize);
					memcpy(buf, m_buffer, fullsize);
					m_cursor = 0;
					std::shared_ptr<wssocket::Message> newmessage = std::make_shared<wssocket::Message>(buf, fullsize);
					return;
				}
			}
		}

	}

	void reset()
	{
		free(m_buffer);
		m_cursor = 0;
	}

private:
	ssize_t m_header_size;
	ssize_t m_foot_size;
	void* m_buffer;
	ssize_t m_cursor;
};

int main(int argc, char** argv)
{
	wssocket::TCP tcpsocket;
	tcpsocket.set_filter(new IM4399Filter(12, 4));
	tcpsocket.set_on_message([&](std::shared_ptr<wssocket::Connection> con, std::shared_ptr<wssocket::Message> msg) -> void {
		printf("receive:%s",(char*)msg->buf, msg->len);
		return;
	});
	tcpsocket.set_on_close([&](std::shared_ptr<wssocket::Connection> contection, const int status, const std::string& reason) -> void {
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