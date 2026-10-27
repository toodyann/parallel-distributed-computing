// 1.2.3 - потік Notify пробуджує лише один із трьох потоків через notify_one
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex notify_mutex;
std::condition_variable notify_cv;
std::condition_variable winner_cv;
int ready = 0;
bool stop_waiting = false;
int winner = 0;

void WaitThread(int id) {
    std::unique_lock<std::mutex> lock(notify_mutex);
    notify_cv.wait(lock, [] { return ready == 1 || stop_waiting; });
    if (ready == 1 && winner == 0) {
        winner = id;
        std::cout << "Message from Thread" << id << "\n";
        winner_cv.notify_one();
    }
}

void Notify() {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    {
        std::lock_guard<std::mutex> lock(notify_mutex);
        ready = 1;
    }
    std::cout << "Notify: notify_one()\n";
    notify_cv.notify_one();
}

int main() {
    std::cout << "TASK 3 - notify_one chooses one waiting thread\n";
    std::vector<std::thread> workers;
    for (int id = 1; id <= 3; ++id) workers.emplace_back(WaitThread, id);
    std::thread notifier(Notify);
    notifier.join();
    {
        // Чекаємо, поки пробуджений потік виведе повідомлення,
        // потім коректно завершуємо решту потоків.
        std::unique_lock<std::mutex> lock(notify_mutex);
        winner_cv.wait(lock, [] { return winner != 0; });
        ready = 0;
        stop_waiting = true;
    }
    notify_cv.notify_all();
    for (auto& thread : workers) thread.join();
    std::cout << "Main: selected Thread" << winner << "; remaining threads closed safely\n";
}
