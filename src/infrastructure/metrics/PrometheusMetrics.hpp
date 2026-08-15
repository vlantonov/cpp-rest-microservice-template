#pragma once
#include "domain/ports/IMetrics.hpp"
#include "domain/ports/IReadinessChecker.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

namespace infrastructure::metrics {

class PrometheusMetrics final
    : public domain::ports::IMetrics
    , public domain::ports::IReadinessChecker
{
public:
    PrometheusMetrics();

    void recordRequest(std::string_view method,
                       std::string_view route,
                       std::string_view status) noexcept override;

    void recordLatency(std::string_view method,
                       std::string_view route,
                       std::chrono::microseconds duration) noexcept override;

    void setActiveConnections(int count) noexcept override;

    [[nodiscard]] std::string serialize() const override;

    [[nodiscard]] bool isReady() const noexcept override;

private:
    prometheus::Registry registry_;
    prometheus::Family<prometheus::Counter>&   requests_total_;
    prometheus::Family<prometheus::Histogram>& request_latency_;
    prometheus::Gauge*                         active_connections_;
};

} // namespace infrastructure::metrics
