#include "infrastructure/logging/SpdlogLogger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace infrastructure::logging {

namespace {

spdlog::level::level_enum to_spdlog_level(domain::ports::ILogger::Level lvl) noexcept {
    using L = domain::ports::ILogger::Level;
    switch (lvl) {
        case L::Trace:    return spdlog::level::trace;
        case L::Debug:    return spdlog::level::debug;
        case L::Info:     return spdlog::level::info;
        case L::Warn:     return spdlog::level::warn;
        case L::Error:    return spdlog::level::err;
        case L::Critical: return spdlog::level::critical;
    }
    return spdlog::level::info;
}

std::string_view level_name(domain::ports::ILogger::Level lvl) noexcept {
    using L = domain::ports::ILogger::Level;
    switch (lvl) {
        case L::Trace:    return "trace";
        case L::Debug:    return "debug";
        case L::Info:     return "info";
        case L::Warn:     return "warn";
        case L::Error:    return "error";
        case L::Critical: return "critical";
    }
    return "info";
}

std::string iso_utc_now() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm gmt{};
    gmtime_r(&tt, &gmt);
    std::ostringstream oss;
    oss << std::put_time(&gmt, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

} // anonymous namespace

SpdlogLogger::SpdlogLogger(const std::string& service_name, const std::string& level)
    : service_name_(service_name)
{
    auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    logger_ = std::make_shared<spdlog::logger>("microservice", sink);
    logger_->set_pattern("%v");  // we emit fully-formed JSON; suppress spdlog wrapper

    // Map level string to spdlog enum
    const auto mapped = spdlog::level::from_str(level);
    logger_->set_level(mapped == spdlog::level::off ? spdlog::level::info : mapped);
}

void SpdlogLogger::log(Level level,
                       std::string_view message,
                       std::string_view trace_id) noexcept {
    const std::string_view tid =
        trace_id.empty() ? std::string_view{"00000000000000000000000000000000"} : trace_id;

    // Build NDJSON manually to avoid fmt/json library dependency in noexcept context.
    // Note: message content is not JSON-escaped (scaffold limitation).
    std::string record;
    record.reserve(256);
    record += R"({"timestamp":")";
    record += iso_utc_now();
    record += R"(","level":")";
    record += level_name(level);
    record += R"(","msg":")";
    record += message;
    record += R"(","service":")";
    record += service_name_;
    record += R"(","trace_id":")";
    record += tid;
    record += R"("})";

    logger_->log(to_spdlog_level(level), record);
}

} // namespace infrastructure::logging
