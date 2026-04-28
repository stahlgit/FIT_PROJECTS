

#include "task_tracker.hpp"

void TaskTracker::register_task(const ScanKey& key, std::promise<PortState>* promise) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    pending_tasks_[key] = promise;
}

void TaskTracker::resolve_task(const ScanKey& key, PortState state) {
    std::promise<PortState>* promise = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(map_mutex_);
        auto it = pending_tasks_.find(key);
        if (it != pending_tasks_.end()) {
            promise = it->second;
            pending_tasks_.erase(it); // Remove it so we don't resolve it twice
        }
    }

    // Set the value OUTSIDE the lock to prevent blocking other threads 
    // while the worker thread wakes up.
    if (promise) {
        try {
            promise->set_value(state);
        } catch (...) {
            // Promise might have been broken/abandoned, safe to ignore
        }
    }
}

void TaskTracker::remove_task(const ScanKey& key) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    pending_tasks_.erase(key);
}