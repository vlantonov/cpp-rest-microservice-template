#include "infrastructure/http/HealthController.hpp"

#include <drogon/HttpResponse.h>
#include <json/json.h>

namespace infrastructure::http {

HealthController::HealthController(domain::ports::IReadinessChecker& checker)
    : checker_(checker)
{}

void HealthController::handleLive(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value body;
    body["status"] = "UP";
    callback(drogon::HttpResponse::newHttpJsonResponse(body));
}

void HealthController::handleReady(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value body;
    if (checker_.isReady()) {
        body["status"] = "READY";
        callback(drogon::HttpResponse::newHttpJsonResponse(body));
    } else {
        body["status"] = "NOT_READY";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k503ServiceUnavailable);
        callback(resp);
    }
}

} // namespace infrastructure::http
