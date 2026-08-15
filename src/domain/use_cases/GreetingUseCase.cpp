#include "domain/use_cases/GreetingUseCase.hpp"

namespace domain::use_cases {

GreetingUseCase::GreetingUseCase(ports::ILogger&  logger,
                                 ports::IMetrics& metrics,
                                 ports::ITracer&  tracer)
    : logger_(logger)
    , metrics_(metrics)
    , tracer_(tracer)
{}

model::GreetingResponse GreetingUseCase::execute(const model::GreetingRequest& request) {
    auto span = tracer_.startSpan("GreetingUseCase::execute");
    const auto trace_id = span->traceId();

    logger_.info("Processing greeting for: " + request.name(), trace_id);

    model::GreetingResponse response{"Hello, " + request.name() + "!"};

    metrics_.recordRequest("greet", "GreetingUseCase::execute", "success");

    span->setStatus(true);
    span->end();

    return response;
}

} // namespace domain::use_cases
