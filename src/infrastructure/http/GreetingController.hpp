#pragma once
#include "domain/ports/ILogger.hpp"
#include "domain/ports/IMetrics.hpp"
#include "domain/ports/ITracer.hpp"
#include "domain/use_cases/GreetingUseCase.hpp"

#include <drogon/HttpController.h>

#include <functional>
#include <memory>

namespace infrastructure::http {

class GreetingController
    : public drogon::HttpController<GreetingController, false>
{
public:
    GreetingController(domain::use_cases::GreetingUseCase& use_case,
                       domain::ports::ITracer&             tracer,
                       domain::ports::ILogger&             logger,
                       domain::ports::IMetrics&            metrics);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GreetingController::handleGreet, "/api/v1/greet", drogon::Get);
    METHOD_LIST_END

    void handleGreet(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    domain::use_cases::GreetingUseCase& use_case_;
    domain::ports::ITracer&             tracer_;
    domain::ports::ILogger&             logger_;
    domain::ports::IMetrics&            metrics_;
};

} // namespace infrastructure::http
