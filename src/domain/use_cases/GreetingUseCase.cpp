#include "domain/use_cases/GreetingUseCase.hpp"
#include <chrono>

namespace domain::use_cases {

GreetingUseCase::GreetingUseCase(ports::ILogger&  logger,
                                 ports::IMetrics& metrics,
                                 ports::ITracer&  tracer)
    : logger_(logger)
    , metrics_(metrics)
    , tracer_(tracer)
{}

model::GreetingResponse GreetingUseCase::execute(const model::GreetingRequest& request) {
    const auto start = std::chrono::steady_clock::now();
    auto span = tracer_.startSpan("GreetingUseCase::execute");
    const auto trace_id = span->traceId();

    logger_.info("Processing greeting for: " + request.name(), trace_id);

    model::GreetingResponse response{"Hello, " + request.name() + "!"};

    metrics_.recordRequest("greet", "GreetingUseCase::execute", "success");
    metrics_.recordLatency("greet", "GreetingUseCase::execute",
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start));

    span->setStatus(true);
    span->end();

    return response;
}

} // namespace domain::use_cases
