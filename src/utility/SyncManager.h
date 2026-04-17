#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <chrono>
#include <optional>

/// A helper class that rate limits calls to "sync" so the rate specified
class SyncManager {
    std::optional<std::chrono::steady_clock::time_point> last_time{};
public:
    SyncManager() = default;

    /// Sync to the rate, that is if this is called faster then the specified rate (in Hz)
    /// then it will block to effectively get that rate.
    void sync(float rate);
};


#endif //SYNC_MANAGER_H
