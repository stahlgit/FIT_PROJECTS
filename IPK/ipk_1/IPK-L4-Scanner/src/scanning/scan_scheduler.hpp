/**
 * @file scan_scheduler.hpp
 * @brief Declaration of the ScanScheduler class for managing concurrent port scanning tasks.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SCAN_SCHEDULER_HPP
#define SCAN_SCHEDULER_HPP

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <memory>

#include "scan_types.hpp"
#include "tcp_scanner.hpp"
#include "udp_scanner.hpp"
#include "pcap_listener.hpp"

static constexpr int MAX_THREADS = 64;

class ScanScheduler
{
public:
  /**
   * @brief Construct scheduler, spinning up the thread pool immediately.
   * @param running Reference to Application's running flag - workers abot cleanly when it goes false.
   * @param timeout_ms Per-port scan timeout passed through to scanners.
   * @param iface Network interface name for pcap/raw socket
   */
  ScanScheduler(std::atomic<bool> &running, int timeout_ms, const std::string &iface);
  ~ScanScheduler();

  // No copy or move - owns threads
  ScanScheduler(const ScanScheduler &) = delete;
  ScanScheduler &operator=(const ScanScheduler &) = delete;

  /**
   * @brief Push task onto the queue.
   */
  void enqueue(ScanTask task);

  /**
   * @brief Signal no more tasks, then block until all workers finish.
   */
  void wait();

private:
  /**
   * @brief Worker thread loop: wait for tasks, execute them, print results. Exits when wait() signals done and queue 
   *  is empty.
   */
  void worker_loop();

  /**
   * @brief Print a scan result to stdout in the required format. Locked to prevent interleaving between threads.
   */
  void print_result(const ScanResult &r) const;

  std::atomic<bool> &running_;
  int timeout_ms_;
  std::string iface_;

  std::deque<ScanTask> queue_;
  std::mutex queue_mutex_;
  std::condition_variable cv_;
  bool done_ = false;

  mutable std::mutex print_mutex_;
  std::vector<std::thread> workers_;

  std::unique_ptr<PcapListener> pcap_listener_;

  // Concurency config:
  // 4 - 32 workers, ideally 2 per core to hide latency but not too many to cause excessive context switching
  static constexpr unsigned int MIN_WORKERS = 4;
  static constexpr unsigned int MAX_WORKERS = 32;
  static constexpr unsigned int WORKERS_PER_CORE = 2;
};

#endif // SCAN_SCHEDULER_HPP
