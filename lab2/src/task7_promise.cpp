// 1.2.7 - передача результатів через promise/future без глобальних змінних
#include <chrono>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

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

int main() {
    std::cout << "TASK 7 - promise/future without globals\nEnter n: ";
    int n;
    std::cin >> n;
    if (n < 1) {
        std::cout << "n must be positive\n";
        return 1;
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
