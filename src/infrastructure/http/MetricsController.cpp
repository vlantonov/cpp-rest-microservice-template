#include "infrastructure/http/MetricsController.hpp"

#include <drogon/HttpResponse.h>

namespace infrastructure::http {

MetricsController::MetricsController(domain::ports::IMetrics& metrics)
    : metrics_(metrics)
{}

void MetricsController::handleMetrics(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(metrics_.serialize());
    // Prometheus text format 0.0.4
    resp->addHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
    callback(resp);
}

} // namespace infrastructure::http
