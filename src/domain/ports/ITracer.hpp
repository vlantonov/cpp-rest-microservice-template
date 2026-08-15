#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace domain::ports {

/// Opaque span handle — infrastructure owns the concrete OTel type.
class ISpan {
public:
    virtual ~ISpan() = default;
    virtual void setAttribute(std::string_view key, std::string_view value) noexcept = 0;
    virtual void setStatus(bool ok, std::string_view description = "") noexcept = 0;
    [[nodiscard]] virtual std::string traceId() const noexcept = 0;
    virtual void end() noexcept = 0;
};

class ITracer {
public:
    virtual ~ITracer() = default;

    /// Extract W3C traceparent from the inbound header and start a child span.
    /// Returns a no-op span when no OTel provider is configured.
    [[nodiscard]] virtual std::unique_ptr<ISpan> startSpan(
        std::string_view operation_name,
        std::string_view traceparent_header = "") noexcept = 0;
};

} // namespace domain::ports
