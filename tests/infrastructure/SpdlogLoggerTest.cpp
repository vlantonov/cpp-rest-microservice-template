#include <catch2/catch_test_macros.hpp>
#include <fakeit/catch/fakeit.hpp>

#include "infrastructure/logging/SpdlogLogger.hpp"
#include "domain/ports/ILogger.hpp"

TEST_CASE("SpdlogLogger — construction", "[infrastructure]") {
    SECTION("constructs without throwing") {
        REQUIRE_NOTHROW(infrastructure::logging::SpdlogLogger("test-service", "info"));
    }

    SECTION("debug level constructs without throwing") {
        REQUIRE_NOTHROW(infrastructure::logging::SpdlogLogger("svc", "debug"));
    }

    SECTION("unknown level falls back to info without throwing") {
        REQUIRE_NOTHROW(infrastructure::logging::SpdlogLogger("svc", "bogus"));
    }
}

TEST_CASE("SpdlogLogger — log() does not throw for all levels", "[infrastructure]") {
    infrastructure::logging::SpdlogLogger logger("test-service", "trace");
    using Level = domain::ports::ILogger::Level;

    SECTION("Trace") { logger.log(Level::Trace,    "trace msg", "trace-id-1"); }
    SECTION("Debug") { logger.log(Level::Debug,    "debug msg"); }
    SECTION("Info")  { logger.log(Level::Info,     "info msg",  "trace-id-2"); }
    SECTION("Warn")  { logger.log(Level::Warn,     "warn msg"); }
    SECTION("Error") { logger.log(Level::Error,    "error msg", "trace-id-3"); }
    SECTION("Critical") { logger.log(Level::Critical, "crit msg"); }
}
