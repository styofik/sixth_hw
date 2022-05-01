#include <iostream>
#include <numeric>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std::chrono;

template <typename Iterator, typename T>
void accumulate_wrapper(Iterator begin, Iterator end, T init, std::atomic<T> &result) {
    result += std::accumulate(begin, end, init);
}

template <typename Iterator, typename T>
T parallel_accumulate(Iterator begin, Iterator end, T init, std::atomic<T> &result) {
    auto size = std::distance(begin, end); // Size of container
    auto num_workers = std::thread::hardware_concurrency(); // Number of threads
    if (size < num_workers * 4) { // Calculations for small containers without threads
        return result += std::accumulate(begin, end, init);
    }

    auto size_per_worker = size / num_workers;
    std::vector<std::thread> threads;

    for (auto i = 0u; i < num_workers - 1; i++) { // Work of additional threads
        threads.push_back(std::thread(
                accumulate_wrapper<Iterator, T>,
                std::next(begin, i * size_per_worker), // begin shift
                std::next(begin, (i + 1) * size_per_worker), // end shift
                0, // init
                std::ref(result))); // result
    }

    result += std::accumulate(
            std::next(begin, (num_workers - 1) * size_per_worker),
            end, init); // Work of main thread

    for (auto &thread : threads) {
        thread.join();
    }
    return result;
}

int main() {
    std::vector<int> sequence(100000); // Concurrency make sense only for 1mln elements and higher

    std::iota(std::begin(sequence), std::end(sequence), 0);

    std::atomic<int> res(0);
    steady_clock::time_point start = steady_clock::now();
    std::cout << parallel_accumulate(std::begin(sequence),
                                     std::end(sequence),
                                     0,
                                     res) 
                << ' ' << duration_cast<nanoseconds>
                    (steady_clock::now() - start).count() << std::endl;

    start = steady_clock::now();
    std::cout << std::accumulate(std::begin(sequence),
                                     std::end(sequence),
                                     0) 
                << ' ' << duration_cast<nanoseconds>
                    (steady_clock::now() - start).count() << std::endl;
}