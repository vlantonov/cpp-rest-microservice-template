#pragma once
#include <chrono>
#include <string>
#include <string_view>

namespace domain::ports {

class IMetrics {
public:
    virtual ~IMetrics() = default;

    /// Increment the request counter for the given method/route/status label set.
    virtual void recordRequest(std::string_view method,
                               std::string_view route,
                               std::string_view status) noexcept = 0;

    /// Observe a request duration sample in the latency histogram.
    virtual void recordLatency(std::string_view method,
                               std::string_view route,
                               std::chrono::microseconds duration) noexcept = 0;

    /// Update the active-connections gauge.
    virtual void setActiveConnections(int count) noexcept = 0;

    /// Render the Prometheus text exposition format 0.0.4 for /metrics.
    [[nodiscard]] virtual std::string serialize() const = 0;
};

} // namespace domain::ports
