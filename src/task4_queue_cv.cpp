// 1.2.4 - програма 1.2.1, де прапорець замінено умовною змінною
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::queue<int> queue_cv;
std::mutex queue_mutex;
std::condition_variable queue_event;
bool queue_done = false;

bool prime(int value) {
    if (value < 2) return false;
    for (int divisor = 2; divisor * divisor <= value; ++divisor)
        if (value % divisor == 0) return false;
    return true;
}

void DataPreparation() {
    int value;
    while (std::cin >> value && value != 0) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            queue_cv.push(value);
        }
        queue_event.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        queue_done = true;
    }
    queue_event.notify_one();
}

void DataProcessing() {
    std::queue<int> primes;
    for (;;) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_event.wait(lock, [] { return !queue_cv.empty() || queue_done; });
        while (!queue_cv.empty()) {
            int value = queue_cv.front();
            queue_cv.pop();
            lock.unlock();
            if (prime(value)) primes.push(value);
            lock.lock();
        }
        if (queue_done) break;
    }
    std::cout << "DataProcessing: primes:";
    while (!primes.empty()) {
        std::cout << ' ' << primes.front();
        primes.pop();
    }
    std::cout << "\n";
}

int main() {
    std::cout << "TASK 4 - queue synchronization with condition_variable\n";
    std::cout << "DataPreparation: enter integers, 0 finishes input\n";
    std::thread preparation(DataPreparation);
    preparation.detach();
    std::thread processing(DataProcessing);
    processing.join();
    std::cout << "Main: queue is empty and input is finished\n";
}
