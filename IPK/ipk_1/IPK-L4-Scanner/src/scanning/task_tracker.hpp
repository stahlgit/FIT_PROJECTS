/**
 * @file task_tracker.hpp
 * @brief Declaration of TaskTracker class, ScanKey, ScanKeyHash and TaskGuard - thread safe registry
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef TASK_TRACKER_HPP
#define TASK_TRACKER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <future>
#include <optional>

#include "scan_types.hpp"

/**
 * @brief Unique identifier for scan task, used as key in TaskTracker
 */
struct ScanKey {
    std::string ip;
    uint16_t port;
    Protocol proto;

    bool operator==(const ScanKey& other) const {
        return ip == other.ip && port == other.port && proto == other.proto;
    }
};

/**
 * @brief Hash function for ScanKey to allow usage in std::unordered_map. Combines hashes of IP, port, and protocol 
 * into a single hash value.
 */
struct ScanKeyHash {
    std::size_t operator()(const ScanKey& k) const {
        std::size_t h1 = std::hash<std::string>{}(k.ip);
        std::size_t h2 = std::hash<uint16_t>{}(k.port);
        std::size_t h3 = std::hash<int>{}(static_cast<int>(k.proto));
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

class TaskTracker {
public:
    static TaskTracker& instance() {
        static TaskTracker tracker;
        return tracker;
    }

    /**
     * @brief Register a new scan and link it to a promise
     * @param key Unique identified for the scan task (IP, protocol, port)
     * @param promise Pointer to the promise that will be fulfilled when the scan result is available
     */
    void register_task(const ScanKey& key, std::promise<PortState>* promise);

    /**
     * @brief Called by the Listener to resolve a pending scan
     * @param key Unique identifier for the scan task (IP, protocol, port)
     * @param state The result of the scan (open/closed/filtered) to set
     */
    void resolve_task(const ScanKey& key, PortState state);

    /**
     * @brief Called by the worker if a timeout occurs (cleans up the map)
     * @param key Unique identifier for the scan task (IP, protocol, port)
     */
    void remove_task(const ScanKey& key);

private:
    TaskTracker() = default;
    ~TaskTracker() = default;
    
    // Prevent copying
    TaskTracker(const TaskTracker&) = delete;
    TaskTracker& operator=(const TaskTracker&) = delete;

    std::mutex map_mutex_;
    std::unordered_map<ScanKey, std::promise<PortState>*, ScanKeyHash> pending_tasks_;
};

/**
 * @brief RAII guard that removes a task from TaskTracker on scope exit. Ensures no dangling promise pointer remain 
 *      even if an exception is thrown.
 */
class TaskGuard {
public:
    TaskGuard(const ScanKey& key, std::promise<PortState>& promise)
        : key_(key)
    {
        TaskTracker::instance().register_task(key_, &promise);
    }
    ~TaskGuard() {
        TaskTracker::instance().remove_task(key_);
    }
    // Non-copyable, non-movable — owns the registration
    TaskGuard(const TaskGuard&) = delete;
    TaskGuard& operator=(const TaskGuard&) = delete;
private:
    ScanKey key_;
};
#endif // TASK_TRACKER_HPP