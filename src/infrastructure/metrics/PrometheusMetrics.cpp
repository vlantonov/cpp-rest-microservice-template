#include "infrastructure/metrics/PrometheusMetrics.hpp"

#include <prometheus/text_serializer.h>

#include <sstream>
#include <string>

namespace infrastructure::metrics {

namespace {
// Bucket boundaries in microseconds: 100µs … 1s
const prometheus::Histogram::BucketBoundaries kLatencyBuckets{
    100, 500, 1'000, 5'000, 10'000, 50'000, 100'000, 500'000, 1'000'000
};
} // anonymous namespace

PrometheusMetrics::PrometheusMetrics()
    : requests_total_(
          prometheus::BuildCounter()
              .Name("http_requests_total")
              .Help("Total number of HTTP requests by method, route, and status")
              .Register(registry_))
    , request_latency_(
          prometheus::BuildHistogram()
              .Name("http_request_duration_microseconds")
              .Help("HTTP request latency histogram in microseconds")
              .Register(registry_))
    , active_connections_(
          &prometheus::BuildGauge()
               .Name("http_active_connections")
               .Help("Current number of active HTTP connections")
               .Register(registry_)
               .Add({}))
{}

void PrometheusMetrics::recordRequest(std::string_view method,
                                      std::string_view route,
                                      std::string_view status) noexcept {
    requests_total_
        .Add({{"method", std::string{method}},
              {"route",  std::string{route}},
              {"status", std::string{status}}})
        .Increment();
}

void PrometheusMetrics::recordLatency(std::string_view method,
                                      std::string_view route,
                                      std::chrono::microseconds duration) noexcept {
    request_latency_
        .Add({{"method", std::string{method}},
              {"route",  std::string{route}}},
             kLatencyBuckets)
        .Observe(static_cast<double>(duration.count()));
}

void PrometheusMetrics::setActiveConnections(int count) noexcept {
    active_connections_->Set(static_cast<double>(count));
}

std::string PrometheusMetrics::serialize() const {
    prometheus::TextSerializer serializer;
    std::ostringstream oss;
    serializer.Serialize(oss, registry_.Collect());
    return oss.str();
}

bool PrometheusMetrics::isReady() const noexcept {
    // Baseline: always ready (no external dependencies in this scaffold).
    // Future iterations should check downstream connectivity here.
    return true;
}

} // namespace infrastructure::metrics
