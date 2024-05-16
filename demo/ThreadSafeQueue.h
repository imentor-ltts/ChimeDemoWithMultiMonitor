#include<queue>
#include<thread>
#include <mutex>
#include <condition_variable>


template <typename T>
class ThreadSafeQueue {
public:
	void push(const T& item) {
		std::lock_guard<std::mutex> lock(mtx);
		queue.push(item);
		cv.notify_one();  // Notify a waiting thread
	}

	T pop() {
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [this] { return !queue.empty(); });  // Wait for an item to be available
		T item = queue.front();
		queue.pop();
		return item;
	}

	bool empty() {
		std::lock_guard<std::mutex> lock(mtx);
		return queue.empty();
	}

private:
	std::queue<T> queue;
	std::mutex mtx;
	std::condition_variable cv;
};

