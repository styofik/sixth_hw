#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>

template<typename T>
class atomic_queue {
private:
    std::queue<T> queue;
    std::mutex mtx;
public:
    atomic_queue() = default;
    ~atomic_queue() = default;

    T& front() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.front();
    }

    T& back() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.back();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }

    void push(T elem) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(elem);
    }

    template<class Arg, class ... Args>
    void emplace(const Arg& arg, const Args&... args) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.emplace(arg, args...);
    };

    void pop() {
        std::lock_guard<std::mutex> lock(mtx);
        queue.pop();
    }


};

void add_in_queue(atomic_queue<int>& q, int begin, int end) {
    for (int i = begin; i < end; i++) {
        q.push(i);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms); // Time for interception for another thread
    }
};

void clear_queue(atomic_queue<int>& q) {
    std::cout << q.size() << std::endl;
    while (not q.empty()) {
        std::cout << q.front() << ' ';
        q.pop();
    }
}

int main() {
    atomic_queue<int> qu;
    int count = 10;
    std::thread first_thread(add_in_queue, std::ref(qu), 0, count);
    std::thread second_thread(add_in_queue, std::ref(qu), count, 2*count);
    add_in_queue(qu, 2*count, 3*count);
    first_thread.join();
    second_thread.join();
    std::cout << qu.size() << std::endl;
    auto qusize = qu.size();

    while (not qu.empty()) {
        std::cout << qu.front() << ' ';
        qu.pop();
    }
    std::cout << std::endl;

}