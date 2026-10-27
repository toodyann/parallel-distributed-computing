// 1.2.6 - черга packaged_task у двох захищених деках та окремий потік-виконавець
#include <condition_variable>
#include <deque>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

long long nth_prime(int n) {
    int found = 0;
    for (long long candidate = 2;; ++candidate) {
        bool is_prime = true;
        for (long long divisor = 2; divisor * divisor <= candidate; ++divisor)
            if (candidate % divisor == 0) { is_prime = false; break; }
        if (is_prime && ++found == n) return candidate;
    }
}

std::deque<std::packaged_task<long long()>> tasks;  // дек завдань
std::deque<int> indices;                            // дек номерів n
std::mutex tasks_mutex;
std::condition_variable tasks_cv;
bool producer_done = false;
std::mutex output_mutex;

void Worker() {
    for (;;) {
        std::packaged_task<long long()> task;
        int n = 0;
        {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            tasks_cv.wait(lock, [] { return !tasks.empty() || producer_done; });
            if (tasks.empty() && producer_done) break;
            task = std::move(tasks.front());
            tasks.pop_front();
            n = indices.front();
            indices.pop_front();
        }
        {
            std::lock_guard<std::mutex> output_lock(output_mutex);
            std::cout << "Worker: calculating prime #" << n << "\n";
        }
        task();
    }
    std::lock_guard<std::mutex> output_lock(output_mutex);
    std::cout << "Worker: all queued tasks completed\n";
}

int main() {
    std::cout << "TASK 6 - packaged_task queue (0 finishes input)\n";
    std::thread worker(Worker);
    std::vector<std::pair<int, std::future<long long>>> results;
    int n;
    while (std::cin >> n && n != 0) {
        if (n < 1) continue;
        std::packaged_task<long long()> task([n] { return nth_prime(n); });
        results.emplace_back(n, task.get_future());
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            indices.push_back(n);
            tasks.push_back(std::move(task));
        }
        tasks_cv.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        producer_done = true;
    }
    tasks_cv.notify_one();
    for (auto& [index, future] : results) {
        long long value = future.get();
        std::lock_guard<std::mutex> output_lock(output_mutex);
        std::cout << "Prime #" << index << " = " << value << "\n";
    }
    worker.join();
    std::cout << "Main: worker joined correctly\n";
}
