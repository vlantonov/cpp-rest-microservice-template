#include "infrastructure/http/GreetingController.hpp"
#include "domain/model/GreetingRequest.hpp"

#include <drogon/HttpResponse.h>
#include <json/json.h>

#include <chrono>
#include <string>

namespace infrastructure::http {

GreetingController::GreetingController(
    domain::use_cases::GreetingUseCase& use_case,
    domain::ports::ITracer&             tracer,
    domain::ports::ILogger&             logger,
    domain::ports::IMetrics&            metrics)
    : use_case_(use_case)
    , tracer_(tracer)
    , logger_(logger)
    , metrics_(metrics)
{}

void GreetingController::handleGreet(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    const auto start = std::chrono::steady_clock::now();

    auto span = tracer_.startSpan("GET /api/v1/greet",
                                   req->getHeader("traceparent"));
    const auto trace_id = span->traceId();

    span->setAttribute("http.method", "GET");
    span->setAttribute("http.route",  "/api/v1/greet");

    const auto name_param = req->getParameter("name");
    if (name_param.empty()) {
        span->setStatus(false, "missing name parameter");
        span->end();

        Json::Value err;
        err["error"] = "missing required parameter: name";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);

        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start);
        metrics_.recordRequest("GET", "/api/v1/greet", "400");
        metrics_.recordLatency("GET", "/api/v1/greet", elapsed);

        callback(resp);
        return;
    }

    auto greet_req = domain::model::GreetingRequest::create(name_param);
    if (!greet_req) {
        span->setStatus(false, "invalid name parameter");
        span->end();

        Json::Value err;
        err["error"] = "invalid name: must be 1-64 printable ASCII characters";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);

        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start);
        metrics_.recordRequest("GET", "/api/v1/greet", "400");
        metrics_.recordLatency("GET", "/api/v1/greet", elapsed);

        callback(resp);
        return;
    }

    logger_.info("Received greeting request", trace_id);
    const auto response = use_case_.execute(*greet_req);

    span->setAttribute("http.status_code", "200");
    span->setStatus(true);
    span->end();

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start);
    metrics_.recordRequest("GET", "/api/v1/greet", "200");
    metrics_.recordLatency("GET", "/api/v1/greet", elapsed);

    Json::Value result;
    result["message"] = response.message();
    callback(drogon::HttpResponse::newHttpJsonResponse(result));
}

} // namespace infrastructure::http
