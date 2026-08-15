# Software Requirements Specification

**Project:** cpp-rest-microservice-template  
**Version:** 1.0.0-draft  
**Date:** 2026-08-16  
**Status:** Draft — awaiting System Architect review

---

## 1. Purpose & Scope

This document specifies the requirements for a **production-grade C++ REST microservice scaffold** intended for use as a portfolio reference and reusable starting point. The scaffold shall demonstrate professional software engineering practices: clean architecture, observability, containerisation, and automated CI quality gates.

The primary audience is the System Architect agent and future contributors. All requirements must be independently testable and unambiguous.

---

## 2. Functional Requirements

### 2.1 HTTP Gateway

| ID | Requirement |
|----|-------------|
| FR-01 | The service shall expose an HTTP server that accepts connections on a configurable TCP port (default 8080). |
| FR-02 | All application routes shall be prefixed with `/api/v1/`. The prefix shall be independently configurable to support future versioning (e.g., `/api/v2/`). |
| FR-03 | The gateway shall return HTTP 404 for any unregistered route and HTTP 405 for a registered route called with an unsupported method. |
| FR-04 | Request and response bodies shall be encoded as `application/json`. |

### 2.2 Health & Readiness Endpoints

| ID | Requirement |
|----|-------------|
| FR-05 | The service shall expose `GET /health/live`. A response of HTTP 200 with body `{"status":"UP"}` shall indicate the process is alive. |
| FR-06 | The service shall expose `GET /health/ready`. A response of HTTP 200 with body `{"status":"READY"}` shall indicate all dependencies (e.g., downstream adapters) are reachable. A response of HTTP 503 with body `{"status":"NOT_READY"}` shall indicate one or more dependencies are unavailable. |
| FR-07 | The liveness and readiness probes shall be reachable even when the application is under load-shedding or circuit-breaking. |

### 2.3 Metrics Endpoint

| ID | Requirement |
|----|-------------|
| FR-08 | The service shall expose `GET /metrics` in the Prometheus text exposition format (version 0.0.4 or OpenMetrics). |
| FR-09 | The metrics endpoint shall publish at minimum: total HTTP requests by method and status code, request latency histogram (p50, p95, p99), and current active connections. |
| FR-10 | The `/metrics` endpoint shall not require authentication in the scaffold baseline; access-control is explicitly out of scope for this iteration. |

### 2.4 Structured JSON Logging

| ID | Requirement |
|----|-------------|
| FR-11 | All log output shall be emitted as newline-delimited JSON (NDJSON) to stdout. |
| FR-12 | Every log record shall include at minimum: `timestamp` (ISO 8601 UTC), `level`, `message`, `service_name`, and `trace_id` (populated from the active OpenTelemetry span when available, otherwise `"00000000000000000000000000000000"`). |
| FR-13 | Log level shall be configurable at runtime via the environment variable `LOG_LEVEL` (accepted values: `trace`, `debug`, `info`, `warn`, `error`, `critical`). The default level shall be `info`. |
| FR-14 | The logging implementation shall use **spdlog** as the underlying library. |

### 2.5 Distributed Tracing

| ID | Requirement |
|----|-------------|
| FR-15 | The service shall instrument all inbound HTTP requests with OpenTelemetry traces using **opentelemetry-cpp**. |
| FR-16 | Each request span shall record: HTTP method, route template, HTTP status code, and server hostname as span attributes conforming to the OpenTelemetry HTTP semantic conventions. |
| FR-17 | The OTLP exporter endpoint shall be configurable via the environment variable `OTEL_EXPORTER_OTLP_ENDPOINT`. When the variable is absent, the SDK shall be configured with a no-op exporter so the service starts without an external collector. |
| FR-18 | Trace context shall be propagated using the W3C `traceparent` / `tracestate` HTTP headers. |

### 2.6 Domain / Infrastructure Layer Separation

| ID | Requirement |
|----|-------------|
| FR-19 | The codebase shall implement a hexagonal (ports-and-adapters) architecture with at least three distinct layers: **domain**, **application**, and **infrastructure**. |
| FR-20 | Domain layer code shall have zero compile-time or link-time dependencies on HTTP, logging, metrics, or tracing libraries. |
| FR-21 | Each external integration point (HTTP server, metrics sink, trace exporter, external service client) shall be represented by a pure abstract interface (port) defined in the application layer. Concrete implementations (adapters) shall reside in the infrastructure layer. |
| FR-22 | The scaffold shall include at least one demonstrative domain use-case (e.g., an `EchoService` or `GreetingService`) that exercises the port/adapter boundary end-to-end. |

### 2.7 Build System

| ID | Requirement |
|----|-------------|
| FR-23 | The project shall be built with **CMake 3.21** or later. The minimum required version shall be declared in `CMakeLists.txt`. |
| FR-24 | Third-party dependencies shall be managed with **Conan 2** (conanfile.py or conanfile.txt). The CMake integration shall use `CMakeDeps` and `CMakeToolchain` generators. |
| FR-25 | The build shall succeed with a single `cmake --preset <preset> && cmake --build --preset <preset>` invocation after running `conan install`. |
| FR-26 | The build system shall expose the following named targets: `<project_name>` (main binary), `<project_name>_tests` (test binary), and `coverage` (coverage data generation). |

### 2.8 Unit Tests

| ID | Requirement |
|----|-------------|
| FR-27 | All unit tests shall be written using **Catch2 v3** (fetched as a header-only or CMake-integrated dependency). |
| FR-28 | Mock objects for ports shall be provided using **FakeIt** v2 or later. |
| FR-29 | All test targets shall be registered with **CTest** so that `ctest --test-dir build` executes the full suite. |
| FR-30 | The test suite shall achieve a minimum line coverage of **70%** as enforced by the CI coverage gate. |
| FR-31 | Tests shall compile and run in isolation from the infrastructure layer; domain and application logic shall be testable via injected mock adapters only. |

### 2.9 Containerisation

| ID | Requirement |
|----|-------------|
| FR-32 | The repository shall include a **multi-stage Dockerfile**. The builder stage shall compile the service; the runtime stage shall copy only the compiled binary and required shared libraries into a minimal base image (e.g., `ubuntu:22.04` or `debian:bookworm-slim`). |
| FR-33 | The runtime image shall not contain compiler toolchain binaries, source files, or build artefacts beyond those needed to run the service. |
| FR-34 | The container shall start the service on port 8080 by default and expose that port via the `EXPOSE` directive. |

### 2.10 Kubernetes Manifests

| ID | Requirement |
|----|-------------|
| FR-35 | The repository shall include a Kubernetes **Deployment** manifest that sets liveness and readiness probes pointing to `GET /health/live` and `GET /health/ready` respectively. |
| FR-36 | The repository shall include a Kubernetes **Service** manifest of type `ClusterIP` exposing port 80 and targeting container port 8080. |
| FR-37 | Both manifests shall use a configurable image tag placeholder (e.g., `IMAGE_TAG`) to enable substitution in CI/CD pipelines. |

### 2.11 GitHub Actions CI

| ID | Requirement |
|----|-------------|
| FR-38 | The repository shall include a GitHub Actions workflow that builds the project with both **GCC** and **Clang** compilers on Ubuntu. |
| FR-39 | The workflow shall use `actions/checkout@v7`. |
| FR-40 | The workflow shall execute four sanitizer builds, each passing the corresponding flags via `-DCMAKE_CXX_FLAGS`: ASAN (`-fsanitize=address`), TSAN (`-fsanitize=thread`), MSAN (`-fsanitize=memory`), and UBSAN (`-fsanitize=undefined`). All sanitizer builds shall also pass `-O1 -g`. |
| FR-41 | The workflow shall run **Valgrind memcheck** against the test binary and fail the job on any reported error. |
| FR-42 | The workflow shall generate a coverage report using **lcov**, producing the file `build/cov.info.cleaned` via a CMake target named `cov_data`. |
| FR-43 | The coverage report shall be published using `zgosalvez/github-actions-report-lcov@v7.2.0`, with a minimum threshold of **70%**. |
| FR-44 | Build artefacts (compiled test binary) shall be shared between jobs using `actions/upload-artifact@v7` (upload) and `actions/download-artifact@v8` (download), consistent with the vladiant/test_cpp_ci pattern. |

---

## 3. Non-Functional Requirements

| ID | Requirement |
|----|-------------|
| NFR-01 | **Language standard:** The codebase shall target **C++20**. The CMake target shall set `CXX_STANDARD 20` and `CXX_STANDARD_REQUIRED ON`. |
| NFR-02 | **Compiler support:** The scaffold shall compile without warnings under GCC 12+ and Clang 16+ with `-Wall -Wextra -Wpedantic`. |
| NFR-03 | **Platform:** Linux (x86-64) is the primary and required platform. macOS compatibility is desirable but not required for this iteration. |
| NFR-04 | **Startup latency:** The service process shall be ready to accept HTTP connections within 5 seconds of process start on commodity hardware. |
| NFR-05 | **Memory safety:** No memory leaks shall be reported by Valgrind memcheck under default test execution. ASAN builds shall exit clean. |
| NFR-06 | **Thread safety:** Any shared state (metrics counters, log sinks, trace exporters) shall be accessed in a thread-safe manner. TSAN builds shall exit clean. |
| NFR-07 | **Dependency freshness:** All Conan-managed dependencies shall be pinned to specific versions in the conanfile; no floating version ranges (e.g., `>=x.y`) shall be used in production pinning. |
| NFR-08 | **Image size:** The final Docker runtime image should not exceed 150 MB compressed. |
| NFR-09 | **License:** All third-party libraries shall be compatible with the project's MIT license. Library licenses shall be documented. |
| NFR-10 | **Reproducibility:** Given the same Conan lock file and CMake preset, two independent builds shall produce bit-for-bit identical binaries (excluding embedded timestamps). |

---

## 4. Constraints & Assumptions

| ID | Constraint / Assumption |
|----|------------------------|
| CA-01 | Conan 2 (not Conan 1) is the only supported package manager. Conan 1 compatibility is not required. |
| CA-02 | CMake presets (`CMakePresets.json`) shall be used to define `debug`, `release`, and `coverage` configurations. |
| CA-03 | The CI runner environment is `ubuntu-latest` (currently ubuntu-22.04 or ubuntu-24.04 as provided by GitHub-hosted runners). |
| CA-04 | MSAN builds require a memory-instrumented C++ standard library. The CI job for MSAN is permitted to use a pre-built libc++ with MSAN instrumentation or to skip the standard library sanitisation and annotate the limitation. |
| CA-05 | The scaffold does not implement business logic beyond a minimal demonstrative use-case; it is a template, not a production service. |
| CA-06 | No persistent storage (database, file system) is required for the scaffold. |
| CA-07 | Authentication and authorisation are explicitly deferred; all endpoints are unauthenticated in this iteration. |
| CA-08 | TLS termination is assumed to be handled by the Kubernetes Ingress or a sidecar proxy; the service itself listens on plain HTTP. |

---

## 5. Out of Scope

- gRPC or GraphQL transport (REST only in this iteration)
- Database connectivity or ORM integration
- Authentication, authorisation, or API keys
- Horizontal pod autoscaling or Kubernetes HPA manifests
- Helm chart packaging
- Rate limiting or circuit-breaker middleware (may be added in a future iteration)
- Windows or macOS CI runners
- Multi-architecture Docker images (arm64 support deferred)
- Integration or end-to-end test suite (unit tests only in scope)
- Secret management (Vault, Kubernetes Secrets injection)

---

## 6. Acceptance Criteria

| ID | Criterion |
|----|-----------|
| AC-01 | `cmake --preset release && cmake --build --preset release` succeeds from a clean checkout after `conan install`. |
| AC-02 | `ctest --test-dir build --output-on-failure` reports 100% tests passing. |
| AC-03 | The GitHub Actions workflow runs all jobs (GCC build, Clang build, ASAN, TSAN, MSAN, UBSAN, Valgrind, coverage) to completion with green status on the `main` branch. |
| AC-04 | Coverage gate reports ≥ 70% line coverage. |
| AC-05 | `docker build -t microservice-template .` produces a runnable image; `docker run -p 8080:8080 microservice-template` responds to `GET /health/live` with HTTP 200. |
| AC-06 | `kubectl apply -f k8s/` applies without validation errors against a Kubernetes 1.28+ API server. |
| AC-07 | Valgrind memcheck reports zero errors on the test binary. |
| AC-08 | The domain layer CMake target links successfully without spdlog, opentelemetry-cpp, or any HTTP library in its dependency graph. |

---

## 7. Glossary

| Term | Definition |
|------|-----------|
| **Adapter** | A concrete implementation of a port; lives in the infrastructure layer. |
| **ASAN** | AddressSanitizer — detects memory corruption bugs. |
| **CTest** | CMake's test runner, invoked via `ctest`. |
| **Domain layer** | Code containing pure business rules with no framework dependencies. |
| **FakeIt** | A header-only C++ mocking framework. |
| **Hexagonal architecture** | Also called ports-and-adapters; separates domain logic from external concerns via interfaces. |
| **lcov** | A Linux test coverage tool that processes gcov data into HTML/info reports. |
| **MSAN** | MemorySanitizer — detects use of uninitialized memory. |
| **NDJSON** | Newline-Delimited JSON; each line is a complete JSON object. |
| **OTLP** | OpenTelemetry Protocol; the wire format for exporting spans, metrics, and logs. |
| **Port** | A pure abstract interface (C++ abstract class) in the application layer. |
| **Prometheus** | An open-source monitoring system with a pull-based metrics model. |
| **spdlog** | A fast, header-friendly C++ logging library. |
| **TSAN** | ThreadSanitizer — detects data races. |
| **UBSAN** | UndefinedBehaviorSanitizer — detects undefined behavior. |
| **W3C traceparent** | An HTTP header standard for distributed trace context propagation. |

---

*Requirements are ready for the System Architect agent to design against.*
