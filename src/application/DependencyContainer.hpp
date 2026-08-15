#pragma once
#include "application/Config.hpp"
#include "infrastructure/logging/SpdlogLogger.hpp"
#include "infrastructure/metrics/PrometheusMetrics.hpp"
#include "infrastructure/tracing/OtelTracer.hpp"
#include "domain/use_cases/GreetingUseCase.hpp"
#include "domain/ports/ILogger.hpp"
#include "domain/ports/IMetrics.hpp"
#include "domain/ports/IReadinessChecker.hpp"
#include "domain/ports/ITracer.hpp"

namespace application {

/// Owns all concrete adapter instances and wires them into use-cases.
/// Members are declared in initialisation order; do not reorder.
class DependencyContainer {
public:
    explicit DependencyContainer(const Config& config)
        : logger_(config.service_name, config.log_level)
        , metrics_()
        , tracer_(config.otel_endpoint, config.service_name)
        , greeting_use_case_(logger_, metrics_, tracer_)
    {}

    domain::ports::ILogger&             logger()          noexcept { return logger_;            }
    domain::ports::IMetrics&            metrics()         noexcept { return metrics_;           }
    domain::ports::ITracer&             tracer()          noexcept { return tracer_;            }
    domain::ports::IReadinessChecker&   readinessChecker()noexcept { return metrics_;           }
    domain::use_cases::GreetingUseCase& greetingUseCase() noexcept { return greeting_use_case_; }

private:
    infrastructure::logging::SpdlogLogger       logger_;
    infrastructure::metrics::PrometheusMetrics  metrics_;
    infrastructure::tracing::OtelTracer         tracer_;
    domain::use_cases::GreetingUseCase          greeting_use_case_;
};

} // namespace application
