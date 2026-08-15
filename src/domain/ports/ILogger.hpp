#pragma once
#include <string_view>

namespace domain::ports {

class ILogger {
public:
    enum class Level { Trace, Debug, Info, Warn, Error, Critical };

    virtual ~ILogger() = default;

    virtual void log(Level level,
                     std::string_view message,
                     std::string_view trace_id = "") noexcept = 0;

    void trace   (std::string_view msg, std::string_view tid = "") noexcept { log(Level::Trace,    msg, tid); }
    void debug   (std::string_view msg, std::string_view tid = "") noexcept { log(Level::Debug,    msg, tid); }
    void info    (std::string_view msg, std::string_view tid = "") noexcept { log(Level::Info,     msg, tid); }
    void warn    (std::string_view msg, std::string_view tid = "") noexcept { log(Level::Warn,     msg, tid); }
    void error   (std::string_view msg, std::string_view tid = "") noexcept { log(Level::Error,    msg, tid); }
    void critical(std::string_view msg, std::string_view tid = "") noexcept { log(Level::Critical, msg, tid); }
};

} // namespace domain::ports
