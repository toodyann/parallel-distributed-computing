// Лабораторна робота №2 - Паралельні та розподілені обчислення
// Синхронізація паралельних операцій за допомогою механізму подій
// та механізму майбутніх результатів.
// Виконав: Коновалов А. С., група КН-41
//
// Усі задачі (1.2.1 - 1.2.7) в одному файлі. Кожна задача - в окремому namespace.
// Збірка:  g++ -std=c++20 -O2 -pthread all_tasks.cpp -o all_tasks
// Запуск:  ./all_tasks <номер 1-7> [deferred|async]   (або без аргументів - меню)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// ============================================================================
// 1.2.1 - синхронізація через глобальний прапорець, unique_lock та sleep_for
// ============================================================================
namespace task1 {

std::queue<int> numbers;
std::mutex data_mutex;
bool input_finished = false;

bool is_prime(int value) {
    if (value < 2) return false;
    for (int divisor = 2; divisor * divisor <= value; ++divisor)
        if (value % divisor == 0) return false;
    return true;
}

void DataPreparation() {
    std::cout << "DataPreparation: enter integers, 0 finishes input\n";
    int value;
    while (std::cin >> value && value != 0) {
        std::lock_guard<std::mutex> lock(data_mutex);
        numbers.push(value);
    }
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        input_finished = true;
    }
    std::cout << "DataPreparation: input completed\n";
}

void DataProcessing() {
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(data_mutex);
            if (input_finished) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::vector<int> primes;
    std::lock_guard<std::mutex> lock(data_mutex);
    while (!numbers.empty()) {
        if (is_prime(numbers.front())) primes.push_back(numbers.front());
        numbers.pop();
    }
    std::cout << "DataProcessing: primes:";
    for (int value : primes) std::cout << ' ' << value;
    std::cout << "\n";
}

void run() {
    std::cout << "TASK 1 - flag polling with sleep_for\n";
    std::thread preparation(DataPreparation);
    preparation.detach();
    std::thread processing(DataProcessing);
    processing.join();
    std::cout << "Main: processing finished\n";
}

}  // namespace task1

// ============================================================================
// 1.2.2 - три потоки Waits() очікують сигнал від Awake() через notify_all
// ============================================================================
namespace task2 {

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

void run() {
    std::cout << "TASK 2 - condition_variable and notify_all\n";
    std::vector<std::thread> waiters;
    for (int id = 1; id <= 3; ++id) waiters.emplace_back(Waits, id);
    std::thread notifier(Awake);
    notifier.join();
    for (auto& thread : waiters) thread.join();
    std::cout << "Main: all waiting threads completed\n";
}

}  // namespace task2

// ============================================================================
// 1.2.3 - потік Notify пробуджує лише один із трьох потоків через notify_one
// ============================================================================
namespace task3 {

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

void run() {
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

}  // namespace task3

// ============================================================================
// 1.2.4 - програма 1.2.1, де прапорець замінено умовною змінною
// ============================================================================
namespace task4 {

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

void run() {
    std::cout << "TASK 4 - queue synchronization with condition_variable\n";
    std::cout << "DataPreparation: enter integers, 0 finishes input\n";
    std::thread preparation(DataPreparation);
    preparation.detach();
    std::thread processing(DataProcessing);
    processing.join();
    std::cout << "Main: queue is empty and input is finished\n";
}

}  // namespace task4

// ============================================================================
// 1.2.5 - обчислення n-го простого числа через std::async (deferred / async)
// ============================================================================
namespace task5 {

long long nth_prime(int n) {
    if (n < 1) return -1;
    int limit = n < 6 ? 15 : static_cast<int>(n * (std::log(n) + std::log(std::log(n)))) + 20;
    for (;;) {
        std::vector<bool> sieve(limit + 1, true);
        sieve[0] = sieve[1] = false;
        for (int p = 2; p * p <= limit; ++p)
            if (sieve[p])
                for (int multiple = p * p; multiple <= limit; multiple += p)
                    sieve[multiple] = false;
        int count = 0;
        for (int value = 2; value <= limit; ++value)
            if (sieve[value] && ++count == n) return value;
        limit *= 2;
    }
}

void run(bool deferred) {
    std::cout << "TASK 5 - std::async mode: " << (deferred ? "deferred" : "async") << "\n";
    std::cout << "Enter n: ";
    int n;
    std::cin >> n;
    auto start = std::chrono::steady_clock::now();
    auto policy = deferred ? std::launch::deferred : std::launch::async;
    auto result = std::async(policy, nth_prime, n);

    std::cout << "Choose function for n (1=sqrt, 2=sin, 3=ln): ";
    int choice;
    std::cin >> choice;
    double auxiliary = choice == 1 ? std::sqrt(n) : choice == 2 ? std::sin(n) : std::log(n);
    std::cout << std::fixed << std::setprecision(6)
              << "Auxiliary result: " << auxiliary << "\n";

    auto before_get = std::chrono::steady_clock::now();
    long long value = result.get();
    auto finish = std::chrono::steady_clock::now();
    auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(finish - before_get).count();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << n << "-th prime: " << value << "\n"
              << "Wait after auxiliary input: " << wait_ms << " ms\n"
              << "Total time: " << total_ms << " ms\n";
}

}  // namespace task5

// ============================================================================
// 1.2.6 - черга packaged_task у двох захищених деках та окремий потік-виконавець
// ============================================================================
namespace task6 {

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

void run() {
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

}  // namespace task6

// ============================================================================
// 1.2.7 - передача результатів через promise/future без глобальних змінних
// ============================================================================
namespace task7 {

long long nth_prime(int n) {
    int found = 0;
    for (long long candidate = 2;; ++candidate) {
        bool is_prime = true;
        for (long long divisor = 2; divisor * divisor <= candidate; ++divisor)
            if (candidate % divisor == 0) { is_prime = false; break; }
        if (is_prime && ++found == n) return candidate;
    }
}

void FirstWorker(int n, std::promise<long long> first, std::promise<bool> signal,
                 std::promise<long long> tenth) {
    first.set_value(nth_prime(n));
    signal.set_value(true);
    tenth.set_value(nth_prime(n * 10));
}

void SecondWorker(int n, std::future<bool> signal, std::mutex& output_mutex) {
    if (signal.get()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << std::fixed << std::setprecision(6)
                  << "SecondWorker: sqrt(" << n << ") = " << std::sqrt(n) << "\n";
    }
}

void run() {
    std::cout << "TASK 7 - promise/future without globals\nEnter n: ";
    int n;
    std::cin >> n;
    if (n < 1) {
        std::cout << "n must be positive\n";
        return;
    }
    std::promise<long long> first_promise, tenth_promise;
    std::promise<bool> signal_promise;
    auto first_future = first_promise.get_future();
    auto tenth_future = tenth_promise.get_future();
    auto signal_future = signal_promise.get_future();
    std::mutex output_mutex;

    std::thread first(FirstWorker, n, std::move(first_promise),
                      std::move(signal_promise), std::move(tenth_promise));
    std::thread second(SecondWorker, n, std::move(signal_future), std::ref(output_mutex));

    long long first_value = first_future.get();
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Main: prime #" << n << " = " << first_value << "\n";
    }
    long long tenth_value = tenth_future.get();
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Main: prime #" << n * 10 << " = " << tenth_value << "\n";
    }
    first.join();
    second.join();
    std::cout << "Main: both void threads joined\n";
}

}  // namespace task7

// ============================================================================
// Головна функція: вибір задачі за номером
// ============================================================================
int main(int argc, char** argv) {
    int task = 0;
    if (argc > 1) {
        task = std::stoi(argv[1]);
    } else {
        std::cout << "Choose task (1-7): ";
        std::cin >> task;
    }

    switch (task) {
        case 1: task1::run(); break;
        case 2: task2::run(); break;
        case 3: task3::run(); break;
        case 4: task4::run(); break;
        case 5: {
            std::string mode;
            if (argc > 2) {
                mode = argv[2];
            } else {
                std::cout << "Mode (deferred/async): ";
                std::cin >> mode;
            }
            task5::run(mode == "deferred");
            break;
        }
        case 6: task6::run(); break;
        case 7: task7::run(); break;
        default:
            std::cout << "Unknown task number\n";
            return 1;
    }
}
