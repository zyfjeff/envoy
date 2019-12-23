#include "extensions/resource_monitors/fixed_heap/fixed_heap_monitor.h"

#include "envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.h"

#include "common/common/assert.h"
#include "common/memory/stats.h"

namespace Envoy {
namespace Extensions {
namespace ResourceMonitors {
namespace FixedHeapMonitor {

FixedHeapMonitor::FixedHeapMonitor(
    const envoy::config::resource_monitor::fixed_heap::v2alpha::FixedHeapConfig& config,
    std::unique_ptr<Common::MemoryStatsReader> stats)
    : max_heap_(config.max_heap_size_bytes()), stats_(std::move(stats)) {
  ASSERT(max_heap_ > 0);
}

void FixedHeapMonitor::updateResourceUsage(Server::ResourceMonitor::Callbacks& callbacks) {
  const size_t physical = stats_->reservedHeapBytes();
  const size_t unmapped = stats_->unmappedHeapBytes();
  ASSERT(physical >= unmapped);
  const size_t used = physical - unmapped;

  Server::ResourceUsage usage;
  usage.resource_pressure_ = used / static_cast<double>(max_heap_);

  callbacks.onSuccess(usage);
}

} // namespace FixedHeapMonitor
} // namespace ResourceMonitors
} // namespace Extensions
} // namespace Envoy
