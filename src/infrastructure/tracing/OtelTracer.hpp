#pragma once
#include "domain/ports/ITracer.hpp"

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/span.h>

#include <memory>
#include <string>

namespace infrastructure::tracing {

/// Wraps an opentelemetry-cpp Span to satisfy ISpan.
class OtelSpan final : public domain::ports::ISpan {
public:
    explicit OtelSpan(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span);

    void setAttribute(std::string_view key, std::string_view value) noexcept override;
    void setStatus(bool ok, std::string_view description = "") noexcept override;
    [[nodiscard]] std::string traceId() const noexcept override;
    void end() noexcept override;

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
};

/// ITracer adapter.  Installs a no-op provider when OTEL_EXPORTER_OTLP_ENDPOINT
/// is empty; otherwise configures an OTLP HTTP exporter.
class OtelTracer final : public domain::ports::ITracer {
public:
    /// @param otlp_endpoint  e.g. "http://otel-collector:4318"  (empty → no-op)
    /// @param service_name   Recorded as the OTel service.name resource attribute.
    OtelTracer(const std::string& otlp_endpoint, const std::string& service_name);
    ~OtelTracer() override = default;

    [[nodiscard]] std::unique_ptr<domain::ports::ISpan> startSpan(
        std::string_view operation_name,
        std::string_view traceparent_header = "") noexcept override;

private:
    std::string service_name_;
};

} // namespace infrastructure::tracing
