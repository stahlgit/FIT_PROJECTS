/**
 * @file scan_scheduler.cpp
 * @brief Implementation of the ScanScheduler class for managing concurrent port scanning tasks.
 * @author Peter Stahl (xstahl01)
 */

#include <iostream>

#include "scan_scheduler.hpp"
#include <algorithm>


ScanScheduler::ScanScheduler(std::atomic<bool>& running, int timeout_ms, const std::string& iface)
    : running_(running), timeout_ms_(timeout_ms), iface_(iface) {
        // initialization of global listener
        pcap_listener_ = std::make_unique<PcapListener>(iface_, running_, timeout_ms_);
        pcap_listener_->start();

        // workers
        unsigned int hw = std::thread::hardware_concurrency();

        auto n = static_cast<int>(std::clamp(hw * WORKERS_PER_CORE, MIN_WORKERS, MAX_WORKERS)); 
        // look into header for details on these constants
        workers_.reserve(n);
        for (int i = 0; i < n; ++i){
            workers_.emplace_back(&ScanScheduler::worker_loop, this);
        }
    }


ScanScheduler::~ScanScheduler() {
    wait();  // safe to call twice — done_ guard protects it
}

void ScanScheduler::enqueue(ScanTask task) {
    {
        std::lock_guard lock(queue_mutex_);
        queue_.push_back(std::move(task));
    }
    cv_.notify_one();
}

void ScanScheduler::wait() {
    {
        std::lock_guard lock(queue_mutex_);
        if (done_) return;
        done_ = true;
    }
    cv_.notify_all();  // wake all workers so they can exit
    for (auto& t : workers_)
        if (t.joinable()) t.join();

    if (pcap_listener_) {
        pcap_listener_->stop();
    }
}

void ScanScheduler::worker_loop() {
    while(true){
        ScanTask task;
        {
            std::unique_lock lock(queue_mutex_);
            cv_.wait(lock, [this]{
                return !queue_.empty() || done_ || !running_;
            });

            if ((!running_ || done_) && queue_.empty())
                return;

            task = std::move(queue_.front());
            queue_.pop_front();
        }
        try{
            ScanResult result = (task.protocol == Protocol::TCP)
            ? TCPScanner(iface_, timeout_ms_, running_).scan(task)
            : UDPScanner(iface_, timeout_ms_, running_).scan(task);
            if (running_){
                print_result(result);
            }
        } catch (const std::exception& e) {
            std::cerr << "Scan failed for " << task.target.ip_str << ":" << task.port << " — " << e.what() << "\n";
        }
    }
}

void ScanScheduler::print_result(const ScanResult& r) const {
    std::lock_guard lock(print_mutex_);
    std::cout << r.ip_str << " " << r.port << " " << proto_str(r.protocol) << " " << state_str(r.state) << "\n";
}