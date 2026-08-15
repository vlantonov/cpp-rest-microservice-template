# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.2] - 2026-08-16

### Fixed

- Corrected `find_package(Drogon)` capitalisation in `src/infrastructure/CMakeLists.txt` (was `drogon`, lowercase). Conan 2's `CMakeDeps` generator produces `DrogonConfig.cmake` with a capital D; on Linux's case-sensitive filesystem the lowercase call caused a CMake configuration error that broke CI on every fresh build. The linked target name is updated consistently (`Drogon::Drogon` instead of `drogon::drogon`).

## [0.1.1] - 2026-08-16

### Fixed

- Added missing `toolchainFile` field to every configure preset in `CMakePresets.json`; without it CMake could not locate the Conan-generated toolchain at `${sourceDir}/build/<build_type>/generators/conan_toolchain.cmake` and CI configuration failed on every fresh checkout.
- Updated GCC Conan profile (`profiles/gcc`) from `compiler.version=12` to `compiler.version=13` to match the `gcc-13` package available on `ubuntu-latest` GitHub Actions runners.

## [0.1.0] - 2026-08-16 [Released]

### Added

- Hexagonal (ports-and-adapters) architecture with `domain/`, `infrastructure/`, and `application/` layers; domain layer has zero external library dependencies.
- `GreetingUseCase` demonstrating the port/adapter boundary end-to-end (`ILogger`, `IMetrics`, `ITracer` injected via constructor).
- `GreetingController` — `GET /api/v1/greet?name=<name>` returning `{"message":"Hello, <name>!"}` (200) or a validation error (400).
- `HealthController` — `GET /health/live` (200 `{"status":"UP"}`) and `GET /health/ready` (200 `{"status":"READY"}` / 503 `{"status":"NOT_READY"}`).
- `MetricsController` — `GET /metrics` serving Prometheus text exposition format 0.0.4.
- `SpdlogLogger` adapter: structured NDJSON logging to stdout; runtime-configurable level via `LOG_LEVEL` env var.
- `OtelTracer` adapter: OpenTelemetry traces with OTLP HTTP exporter; falls back to a no-op provider when `OTEL_EXPORTER_OTLP_ENDPOINT` is unset.
- `PrometheusMetrics` adapter: request counter, latency histogram, and active-connection gauge; rendered by `MetricsController`.
- `DependencyContainer` wiring all adapters and injecting them into use cases and controllers.
- `Config` struct reading `PORT`, `LOG_LEVEL`, `OTEL_EXPORTER_OTLP_ENDPOINT`, and `SERVICE_NAME` from environment variables.
- CMake 3.21 build system with `CXX_STANDARD 20`, `-Wall -Wextra -Wpedantic`, and separate `microservice_domain`, `microservice_infra`, and `microservice_app` targets.
- Conan 2 dependency management (`conanfile.py`) with pinned versions: Drogon 1.9.5, spdlog 1.14.1, opentelemetry-cpp 1.16.1, prometheus-cpp 1.2.4, Catch2 3.7.1, FakeIt 2.4.1.
- `CMakePresets.json` defining `debug`, `release`, `coverage`, `asan`, `tsan`, `msan`, and `ubsan` presets.
- `lcov`/`genhtml` coverage targets (`cov_data`, `cov`) integrated into the CMake build.
- Unit tests with Catch2 v3 and FakeIt mocks covering `GreetingUseCase` and `SpdlogLogger`; registered with CTest.
- Multi-stage `Dockerfile` (builder: ubuntu:24.04 + GCC 12 + Conan 2; runtime: ubuntu:24.04 with stripped binary).
- Kubernetes `Deployment` and `ClusterIP` `Service` manifests under `deploy/k8s/`; `IMAGE_TAG` placeholder for CI substitution.
- GCC and Clang profiles under `profiles/` for reproducible Conan builds.
- GitHub Actions `build.yml`: builds and tests with GCC 12 and Clang 16 on `push` to `main`/`develop` and PRs to `main`; uploads GCC test binary artefact.
- GitHub Actions `sanitizers.yml`: ASAN, TSAN, and UBSAN jobs (Clang 16) on PRs to `main`.
- GitHub Actions `coverage.yml`: lcov coverage gate (≥ 70%) on `push` to `main`; publishes HTML coverage artefact.
- GitHub Actions `valgrind.yml`: Valgrind memcheck (GCC Debug) on `push` to `main`; uploads memcheck log artefact.
- `docs/requirements/SRS.md` and `docs/design/DESIGN.md` covering functional requirements, non-functional requirements, architecture, interface definitions, and CMake target layout.
- `docs/ci-cd/cpp-rest-microservice-template-pipeline.md` CI/CD pipeline reference.

[Unreleased]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.2...HEAD
[0.1.2]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/vlantonov/cpp-rest-microservice-template/releases/tag/v0.1.0
