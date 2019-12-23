#pragma once

#include "envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.h"
#include "envoy/server/resource_monitor.h"

#include "extensions/resource_monitors/common/memory_stats_reader.h"

namespace Envoy {
namespace Extensions {
namespace ResourceMonitors {
namespace FixedHeapMonitor {

/**
 * Heap memory monitor with a statically configured maximum.
 */
class FixedHeapMonitor : public Server::ResourceMonitor {
public:
  FixedHeapMonitor(
      const envoy::config::resource_monitor::fixed_heap::v2alpha::FixedHeapConfig& config,
      std::unique_ptr<Common::MemoryStatsReader> stats =
          std::make_unique<Common::MemoryStatsReader>());

  void updateResourceUsage(Server::ResourceMonitor::Callbacks& callbacks) override;

private:
  const uint64_t max_heap_;
  std::unique_ptr<Common::MemoryStatsReader> stats_;
};

} // namespace FixedHeapMonitor
} // namespace ResourceMonitors
} // namespace Extensions
} // namespace Envoy
