#pragma once
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace application {

/// Reads service configuration from environment variables.
struct Config {
    int         port         = 8080;
    std::string log_level    = "info";
    std::string otel_endpoint;                      // empty → no-op tracer
    std::string service_name = "cpp-microservice";

    Config() {
        if (const char* p = std::getenv("PORT"); p && *p) {
            port = std::stoi(std::string{p});
        }
        if (const char* p = std::getenv("LOG_LEVEL"); p && *p) {
            log_level = p;
        }
        if (const char* p = std::getenv("OTEL_EXPORTER_OTLP_ENDPOINT"); p && *p) {
            otel_endpoint = p;
        }
        if (const char* p = std::getenv("SERVICE_NAME"); p && *p) {
            service_name = p;
        }
    }
};

} // namespace application
