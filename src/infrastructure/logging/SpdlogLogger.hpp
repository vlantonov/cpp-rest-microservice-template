#pragma once
#include "domain/ports/ILogger.hpp"
#include <memory>
#include <string>

// Forward-declare spdlog logger to keep the header light
namespace spdlog { class logger; }

namespace infrastructure::logging {

class SpdlogLogger final : public domain::ports::ILogger {
public:
    /// @param service_name  Embedded in every log record as "service".
    /// @param level         Minimum level string: trace/debug/info/warn/error/critical.
    SpdlogLogger(const std::string& service_name, const std::string& level);

    void log(Level level,
             std::string_view message,
             std::string_view trace_id = "") noexcept override;

private:
    std::string service_name_;
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace infrastructure::logging
