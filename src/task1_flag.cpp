// 1.2.1 - синхронізація через глобальний прапорець, unique_lock та sleep_for
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

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

int main() {
    std::cout << "TASK 1 - flag polling with sleep_for\n";
    std::thread preparation(DataPreparation);
    preparation.detach();
    std::thread processing(DataProcessing);
    processing.join();
    std::cout << "Main: processing finished\n";
}
