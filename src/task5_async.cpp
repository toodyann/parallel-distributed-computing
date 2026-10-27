// 1.2.5 - обчислення n-го простого числа через std::async (deferred / async)
// Запуск: ./task5_async deferred   або   ./task5_async async
#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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

int main(int argc, char** argv) {
    const bool deferred = argc > 1 && std::string(argv[1]) == "deferred";
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
