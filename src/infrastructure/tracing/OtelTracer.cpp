#include "infrastructure/tracing/OtelTracer.hpp"

// OTel API
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#include <opentelemetry/trace/noop.h>
#include <opentelemetry/trace/span_startoptions.h>
#include <opentelemetry/trace/scope.h>

// OTel SDK (only referenced when OTLP endpoint is configured)
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>

// OTLP HTTP exporter
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>

#include <string>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp      = opentelemetry::exporter::otlp;
namespace nostd     = opentelemetry::nostd;

namespace infrastructure::tracing {

// ---------------------------------------------------------------------------
// OtelSpan
// ---------------------------------------------------------------------------

OtelSpan::OtelSpan(nostd::shared_ptr<trace_api::Span> span)
    : span_(std::move(span))
{}

void OtelSpan::setAttribute(std::string_view key, std::string_view value) noexcept {
    span_->SetAttribute(std::string{key}, std::string{value});
}

void OtelSpan::setStatus(bool ok, std::string_view description) noexcept {
    span_->SetStatus(
        ok ? trace_api::StatusCode::kOk : trace_api::StatusCode::kError,
        std::string{description});
}

std::string OtelSpan::traceId() const noexcept {
    char buf[32] = {};
    span_->GetContext().trace_id().ToLowerBase16({buf, 32});
    return std::string(buf, 32);
}

void OtelSpan::end() noexcept {
    span_->End();
}

// ---------------------------------------------------------------------------
// OtelTracer
// ---------------------------------------------------------------------------

OtelTracer::OtelTracer(const std::string& otlp_endpoint,
                       const std::string& service_name)
    : service_name_(service_name)
{
    if (otlp_endpoint.empty()) {
        // Install no-op provider so the process starts without a collector.
        auto noop = nostd::shared_ptr<trace_api::TracerProvider>(
            new trace_api::NoopTracerProvider());
        trace_api::Provider::SetTracerProvider(std::move(noop));
    } else {
        otlp::OtlpHttpExporterOptions opts;
        // Append the standard OTLP trace path if not already present.
        opts.url = otlp_endpoint;
        if (opts.url.back() == '/') {
            opts.url += "v1/traces";
        } else {
            opts.url += "/v1/traces";
        }

        auto exporter  = otlp::OtlpHttpExporterFactory::Create(opts);
        auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(
            std::move(exporter));

        auto resource = opentelemetry::sdk::resource::Resource::Create(
            {{"service.name", service_name_}});

        auto provider = trace_sdk::TracerProviderFactory::Create(
            std::move(processor), resource);

        trace_api::Provider::SetTracerProvider(
            nostd::shared_ptr<trace_api::TracerProvider>(provider.release()));
    }
}

std::unique_ptr<domain::ports::ISpan> OtelTracer::startSpan(
    std::string_view operation_name,
    [[maybe_unused]] std::string_view traceparent_header) noexcept
{
    // Full W3C traceparent extraction via the OTel propagator API is a future
    // enhancement; for now we always start a root span.
    auto tracer = trace_api::Provider::GetTracerProvider()
                      ->GetTracer(service_name_);
    auto span = tracer->StartSpan(std::string{operation_name});
    return std::make_unique<OtelSpan>(std::move(span));
}

} // namespace infrastructure::tracing
