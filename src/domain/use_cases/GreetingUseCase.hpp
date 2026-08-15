#pragma once
#include "domain/model/GreetingRequest.hpp"
#include "domain/model/GreetingResponse.hpp"
#include "domain/ports/ILogger.hpp"
#include "domain/ports/IMetrics.hpp"
#include "domain/ports/ITracer.hpp"

namespace domain::use_cases {

class GreetingUseCase {
public:
    GreetingUseCase(ports::ILogger& logger,
                    ports::IMetrics& metrics,
                    ports::ITracer& tracer);

    [[nodiscard]] model::GreetingResponse execute(const model::GreetingRequest& request);

private:
    ports::ILogger&  logger_;
    ports::IMetrics& metrics_;
    ports::ITracer&  tracer_;
};

} // namespace domain::use_cases
