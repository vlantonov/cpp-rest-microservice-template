# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed

- **Docker build — `--output-folder` removed from `conan install`**: The flag conflicted with `cmake_layout()` in `conanfile.py`; generators were landing directly in `build/Release/` instead of `build/Release/generators/`, causing `cmake --preset release` to fail with "toolchain file not found".
- **Docker build — GCC updated from gcc-12 to Ubuntu 24.04 default (gcc-13)**: The Dockerfile was installing the version-pinned `gcc-12`/`g++-12` packages, which diverged from `profiles/gcc` (`compiler.version=13`). Switched to the `gcc`/`g++` meta-packages (default gcc-13 on Ubuntu 24.04) and removed the now-unnecessary `update-alternatives` block.
- **Docker build — `conan install` now uses the project profile**: Previously the build used inline `-s compiler.*` flags, bypassing the `tools.build:compiler_executables` entry in `profiles/gcc`. The Dockerfile now copies `profiles/gcc` over `~/.conan2/profiles/default` before running `conan install`, ensuring the generated `conan_toolchain.cmake` sets `CMAKE_CXX_COMPILER` explicitly.

- **Test link failure — `undefined reference to test_main`**: Making `Drogon::Drogon` PUBLIC on `microservice_infra` caused the test executable to transitively link Drogon's full dependency closure, including `libboost_test_exec_monitor.a` (Boost.Test), which requires a `test_main(int, char**)` symbol that Catch2 never provides. Fixed by reverting `Drogon::Drogon` to `PRIVATE` on `microservice_infra` and adding it as a direct `PRIVATE` dependency of `microservice_app` (the only target that includes `<drogon/drogon.h>` and the HTTP controller headers). Also suppressed two `-Wunused-result` warnings for intentional `[[nodiscard]]` discards in side-effect test cases.
- **Clang CI — cmake configure used GCC instead of Clang**: The Clang profile's `[buildenv]` set `CC`/`CXX` only for `conan install`; the generated `conan_toolchain.cmake` did not contain `CMAKE_CXX_COMPILER`, so `cmake --preset` picked up the system default `/usr/bin/c++` (GCC). CMake then received the Clang-generated toolchain flags (including `-stdlib=libstdc++`) and drove GCC, which rejected them. Fixed by adding `tools.build:compiler_executables` to both `profiles/clang` and `profiles/gcc`; Conan 2's `CMakeToolchain` writes that key as `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` into the toolchain file, making the compiler explicit. Also removed duplicate `CC`/`CXX` entries in `profiles/clang`.
- **GCC CI — spurious `-stdlib=libstdc++` flag**: Conan's `CMakeToolchain` was emitting `-stdlib=libstdc++` for the GCC profile because `compiler.libcxx=libstdc++11` was set. Fixed by removing the `libcxx`-based toolchain block for GCC in `conanfile.py` and asserting `_GLIBCXX_USE_CXX11_ABI=1` directly via a preprocessor definition instead.
- **Clang CI — `prometheus/counter.h` not found**: `prometheus-cpp::core` was linked `PRIVATE` to `microservice_infra`, so its include directories were not propagated to `microservice_app`. Fixed by changing the linkage to `PUBLIC` in `src/infrastructure/CMakeLists.txt`.
- **App target — `opentelemetry/nostd/shared_ptr.h` not found**: `opentelemetry-cpp::opentelemetry-cpp` was linked `PRIVATE` to `microservice_infra`, so its include directories were not propagated to `microservice_app`, which transitively includes `OtelTracer.hpp` via `DependencyContainer.hpp`. Fixed by changing the linkage to `PUBLIC` in `src/infrastructure/CMakeLists.txt`.
- **App target — `drogon/HttpController.h` not found** (latent): `Drogon::Drogon` was linked `PRIVATE` to `microservice_infra`, so its include directories were not propagated to `microservice_app`, which includes the three controller headers that in turn include `<drogon/HttpController.h>`. Fixed by changing the linkage to `PUBLIC` in `src/infrastructure/CMakeLists.txt`.
- **Both CI matrices — `fakeit/catch/fakeit.hpp` not found**: The FakeIt Catch2 integration header moved to `fakeit/catch2/fakeit.hpp` in recent releases; the old path no longer exists in the installed package. Fixed by updating the include to `<fakeit/catch2/fakeit.hpp>` in the test sources. Corrected the corresponding reference in `docs/design/DESIGN.md`.
- **Test targets — `fakeit/catch2/fakeit.hpp` not found**: The Conan Center recipe for `fakeit/2.4.1` installs headers directly under `include/` without a `fakeit/` subdirectory prefix. The correct include is `<catch2/fakeit.hpp>`. Fixed in `tests/domain/GreetingUseCaseTest.cpp`; removed unused FakeIt include entirely from `tests/infrastructure/SpdlogLoggerTest.cpp`. Corrected the corresponding reference in `docs/design/DESIGN.md`.
- **Test targets — `catch2/fakeit.hpp` not found**: FakeIt 2.4.x removed all per-framework subdirectory headers and ships a single amalgamated `fakeit.hpp` that auto-detects the test framework. Neither `catch2/fakeit.hpp` nor the earlier `fakeit/catch2/fakeit.hpp` paths exist in the installed package. Fixed by changing the include to `<fakeit.hpp>` in `tests/domain/GreetingUseCaseTest.cpp`.

## [0.1.3] - 2026-08-16

### Fixed

- Replaced non-existent `opentelemetry-cpp::opentelemetry_api`, `opentelemetry-cpp::opentelemetry_sdk`, and `opentelemetry-cpp::opentelemetry_exporter_otlp_http` targets in `src/infrastructure/CMakeLists.txt` with the aggregate `opentelemetry-cpp::opentelemetry-cpp`. The Conan 2 `CMakeDeps` generator for `opentelemetry-cpp/1.16.1` does not produce per-library component targets with those names (the recipe defines components like `opentelemetry_trace`, `opentelemetry_common`, etc., but not `opentelemetry_api` or `opentelemetry_sdk`); CMake found the package config but failed at the generate step when resolving the missing IMPORTED targets. Using the recipe-documented aggregate target resolves the error across all four CI workflows (Release, Debug/Coverage, RelWithDebInfo/Sanitizers, Debug/Valgrind).

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

[Unreleased]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.3...HEAD
[0.1.3]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/vlantonov/cpp-rest-microservice-template/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/vlantonov/cpp-rest-microservice-template/releases/tag/v0.1.0
