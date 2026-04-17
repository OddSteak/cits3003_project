#include "SyncManager.h"

#include <thread>

using namespace std::chrono_literals;

void SyncManager::sync(float rate) {
    auto start = std::chrono::steady_clock::now();
    if (!last_time.has_value()) {
        last_time = start;
        return;
    }

    auto interval = 1s / rate;

    while (std::chrono::steady_clock::now() - last_time.value() < interval) {
        std::this_thread::yield();
    }

    last_time = std::chrono::steady_clock::now();
}
