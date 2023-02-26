#pragma once

#include <mutex>
#include <queue>

namespace Pit {
	// Thread safe queue
	template<typename T>
	class TSQueue {
	public:
		TSQueue() = default;
		TSQueue(const TSQueue<T>&) = delete;
		virtual ~TSQueue() { clear(); }

		T& front() {
			std::scoped_lock lock(m_MuxQueue);
			return m_Dequeue.front();
		}

		T& back() {
			std::scoped_lock lock(m_MuxQueue);
			return m_Dequeue.back();
		}

		T pop_front() {
			std::scoped_lock lock(m_MuxQueue);
			auto t = std::move(m_Dequeue.front());
			m_Dequeue.pop_front();
			return t;
		}

		T pop_back() {
			std::scoped_lock lock(m_MuxQueue);
			auto t = std::move(m_Dequeue.back());
			m_Dequeue.pop_back();
			return t;
		}

		void push_back(const T& item) {
			std::scoped_lock lock(m_MuxQueue);
			m_Dequeue.emplace_back(std::move(item));

			std::unique_lock<std::mutex> ul(m_MuxBlocking);
			m_CVBlocking.notify_one();
		}

		void push_front(const T& item) {
			std::scoped_lock lock(m_MuxQueue);
			m_Dequeue.emplace_front(std::move(item));

			std::unique_lock<std::mutex> ul(m_MuxBlocking);
			m_CVBlocking.notify_one();
		}

		template<typename... Args>
		void emplace_front(Args&&... args) {
			std::scoped_lock lock(m_MuxQueue);
			m_Dequeue.emplace_front(std::forward<Args>(args)...);

			std::unique_lock<std::mutex> ul(m_MuxBlocking);
			m_CVBlocking.notify_one();
		}

		template<typename... Args>
		void emplace_back(Args&&... args) {
			std::scoped_lock lock(m_MuxQueue);
			m_Dequeue.emplace_back(std::forward<Args>(args)...);

			std::unique_lock<std::mutex> ul(m_MuxBlocking);
			m_CVBlocking.notify_one();
		}

		bool empty() {
			std::scoped_lock lock(m_MuxQueue);
			return m_Dequeue.empty();
		}

		size_t count() {
			std::scoped_lock lock(m_MuxQueue);
			return m_Dequeue.size();
		}

		void clear() {
			std::scoped_lock lock(m_MuxQueue);
			m_Dequeue.clear();
		}

		void wait() {
			while (empty()) {
				std::unique_lock<std::mutex> ul(m_MuxBlocking);
				m_CVBlocking.wait(ul);
			}
		}

	protected:
		std::mutex m_MuxQueue;
		std::deque<T> m_Dequeue;
		std::condition_variable m_CVBlocking;
		std::mutex m_MuxBlocking;
	};
}