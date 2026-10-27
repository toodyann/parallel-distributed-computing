// 1.2.2 - три потоки Waits() очікують сигнал від Awake() через notify_all
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex event_mutex;
std::condition_variable event_cv;
int event_value = 0;

void Waits(int id) {
    std::unique_lock<std::mutex> lock(event_mutex);
    std::cout << "Waits " << id << ": entering wait state\n";
    event_cv.wait(lock, [] { return event_value == 1; });
    std::cout << "Waits " << id << ": waiting completed\n";
}

void Awake() {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    std::cout << "Awake: first notify_all, predicate is still false\n";
    event_cv.notify_all();
    std::this_thread::sleep_for(100ms);
    {
        std::lock_guard<std::mutex> lock(event_mutex);
        event_value = 1;
    }
    std::cout << "Awake: event_value=1, second notify_all\n";
    event_cv.notify_all();
}

int main() {
    std::cout << "TASK 2 - condition_variable and notify_all\n";
    std::vector<std::thread> waiters;
    for (int id = 1; id <= 3; ++id) waiters.emplace_back(Waits, id);
    std::thread notifier(Awake);
    notifier.join();
    for (auto& thread : waiters) thread.join();
    std::cout << "Main: all waiting threads completed\n";
}
