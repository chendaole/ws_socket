//
// Created by 4399-1500 on 2019/2/11.
//

#pragma once

#include <thread>
#include <functional>
#include <future>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <exception>
#include <chrono>

namespace wssocket
{
	class ThreadPool {
	public:
		typedef std::function<void()> Task;

	public:
		ThreadPool(unsigned int size) {
			m_idle_num = size < 1 ? 1 : size;

			for (size = 0; size < m_idle_num; ++size) {
				m_pool.emplace_back([this]() {
					while (!this->m_stoped) {
						Task task;
						{
							while (this->m_tasks.empty())
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(200));
							}
							if (this->m_stoped) {
								return;
							}
							//将任务移动出来
							task = std::move(this->m_tasks.front());
							this->m_tasks.pop();
						}

						m_idle_num--;
						task();
						m_idle_num++;
					}
					});
			}
		};
		~ThreadPool()
		{
			m_stoped.store(true);
			for (std::thread& t : m_pool) {
				t.join();
			}
		}

		template <typename F, typename... Args>
		auto Commit(F&& f, Args&& ... args) -> std::future<decltype(f(args...))> {
			typedef decltype(f(args...)) RetType;
			auto task = std::make_shared<std::packaged_task<RetType()> >(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

			std::future<RetType> fut = task->get_future();
			{
				std::lock_guard<std::mutex> lock(this->m_mtx);
				m_tasks.emplace([task]() {
					(*task)();
					return;
				});
			}
			return fut;
		}
		int idlNum() { return m_idle_num; };

	private:
		std::mutex              m_mtx;
		std::queue<Task>        m_tasks;
		std::vector<std::thread> m_pool;
		std::atomic<bool>       m_stoped;
		std::atomic<int>        m_idle_num;
	};
}

