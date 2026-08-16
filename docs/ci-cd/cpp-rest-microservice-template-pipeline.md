# cpp-rest-microservice-template — CI/CD Pipeline

**Project:** cpp-rest-microservice-template  
**Ref:** DESIGN.md §9 CI Job Topology  
**Workflows:** `.github/workflows/`

---

## Workflows

| File | Trigger | Purpose |
|---|---|---|
| `build.yml` | push→main/develop, PR→main | GCC + Clang Release build and unit tests (matrix) |
| `sanitizers.yml` | PR→main, manual | ASAN / TSAN / UBSAN on Clang 16 |
| `valgrind.yml` | push→main, manual | Valgrind memcheck on GCC Debug build |
| `coverage.yml` | push→main, manual | lcov coverage + zgosalvez report; gate ≥ 70 % |

---

## Job dependency graph

```
push to main / develop          pull_request to main
        │                               │
        ▼                               ▼
┌──────────────┐  ┌───────────────┐   ┌──────┐ ┌──────┐ ┌──────┐
│  build (gcc) │  │ build (clang) │   │ asan │ │ tsan │ │ubsan │
│   Release    │  │   Release     │   └──────┘ └──────┘ └──────┘
└──────┬───────┘  └───────────────┘     (each rebuilds from source
       │                                 with RelWithDebInfo + preset)
       │ upload-artifact@v7
       │ (test_binary_gcc, 3-day TTL)
       │
push to main only
       │
       ├──▶ valgrind.yml  (Debug build from scratch + memcheck)
       └──▶ coverage.yml  (Coverage build; cov_data target; report-lcov)
```

---

## GitLab CI Pipeline

**Pipeline file:** `.gitlab-ci.yml`  
Three stages execute in order: `build` → `test` → `quality`. Independent jobs within a stage run in parallel.

`default: interruptible: true` cancels in-flight runs when a newer commit arrives on the same ref — equivalent to GitHub Actions `concurrency: cancel-in-progress: true`.

### Jobs

| Job | Stage | Trigger | Purpose |
|---|---|---|---|
| `docker-build` | build | push→main/develop, MR→main | `docker build --progress=plain .`; unit tests run inside the builder stage via `ctest` — a test failure aborts the Docker build |
| `build-gcc` | build | push→main/develop, MR→main | GCC Release build + `ctest` |
| `build-clang` | build | push→main/develop, MR→main | Clang 16 Release build + `ctest` |
| `sanitizer-asan` | test | MR→main (auto), manual (allow\_failure) | ASAN on Clang 16 RelWithDebInfo |
| `sanitizer-tsan` | test | MR→main (auto), manual (allow\_failure) | TSAN on Clang 16 RelWithDebInfo |
| `sanitizer-ubsan` | test | MR→main (auto), manual (allow\_failure) | UBSAN on Clang 16 RelWithDebInfo |
| `valgrind` | quality | push→main (auto), manual (allow\_failure) | Valgrind memcheck; artifact `valgrind-memcheck.txt` (2 weeks) |
| `coverage` | quality | push→main (auto), manual (allow\_failure) | lcov ≥ 70 % gate; artifact `build/Coverage/cov.info.cleaned` (1 week) |

### Stage diagram

```
push to main / develop                  MR → main
        │                                    │
        ▼                                    ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ docker-build │  │  build-gcc   │  │ build-clang  │
│   (DinD)     │  │   Release    │  │   Release    │
└──────────────┘  └──────────────┘  └──────────────┘

              MR → main (auto) / manual (allow_failure)
                             │
             ┌───────────────┼───────────────┐
             ▼               ▼               ▼
     ┌───────────────┐ ┌──────────────┐ ┌───────────────┐
     │sanitizer-asan │ │sanitizer-tsan│ │sanitizer-ubsan│
     └───────────────┘ └──────────────┘ └───────────────┘

            push to main (auto) / manual (allow_failure)
                             │
                    ┌────────┴────────┐
                    ▼                 ▼
             ┌──────────┐     ┌──────────┐
             │ valgrind │     │ coverage │
             └──────────┘     └──────────┘
```

### Caching strategy

`CONAN_HOME` is set to `$CI_PROJECT_DIR/.conan2-cache` so all Conan-managed paths fall inside the workspace directory and are captured by GitLab's native cache mechanism. Each job uses `policy: pull-push`.

| Cache key | Fallback key | Used by |
|---|---|---|
| `conan-gcc-release-<ref>` | `conan-gcc-release-` | `build-gcc` |
| `conan-clang-release-<ref>` | `conan-clang-release-` | `build-clang` |
| `conan-clang-asan-<ref>` | `conan-clang-asan-` | `sanitizer-asan` |
| `conan-clang-tsan-<ref>` | `conan-clang-tsan-` | `sanitizer-tsan` |
| `conan-clang-ubsan-<ref>` | `conan-clang-ubsan-` | `sanitizer-ubsan` |
| `conan-gcc-debug-<ref>` | `conan-gcc-debug-` | `valgrind`, `coverage` (shared) |

The `fallback_keys` entry drops `$CI_COMMIT_REF_SLUG` from the key, so a new branch inherits the last warm cache from any previous run sharing the same compiler and build-type combination.

### Docker-in-Docker (`docker-build`)

`docker-build` runs on `image: docker:27` with a `docker:27-dind` service. TLS is enforced via `DOCKER_TLS_CERTDIR: "/certs"`. Setting `DOCKER_BUILDKIT: "1"` enables BuildKit so the `# syntax=docker/dockerfile:1` frontend directive in the `Dockerfile` takes effect and `--mount=type=cache` is honoured inside the build.

### Coverage badge

The `coverage` job sets `coverage: '/lines\.*: (\d+\.\d+)%/'` so GitLab parses the line-coverage percentage from the `lcov --summary` output and surfaces it as a pipeline badge. A `python3` one-liner in the job script fails the job if line coverage is below 70 %.

---

## Build system layout (Conan 2 + CMakePresets.json)

`conanfile.py` uses `cmake_layout(self)`, which places generated files under
`build/<build_type>/`. The presets in `CMakePresets.json` are aligned with this
layout:

| Preset | binaryDir | Toolchain file |
|---|---|---|
| `release` | `build/Release` | `build/Release/conan_toolchain.cmake` |
| `debug` | `build/Debug` | `build/Debug/conan_toolchain.cmake` |
| `coverage` | `build/Coverage` | `build/Debug/conan_toolchain.cmake` |
| `asan` | `build/ASAN` | `build/RelWithDebInfo/conan_toolchain.cmake` |
| `tsan` | `build/TSAN` | `build/RelWithDebInfo/conan_toolchain.cmake` |
| `ubsan` | `build/UBSAN` | `build/RelWithDebInfo/conan_toolchain.cmake` |

`cmake_layout` sets `generators_folder = build/<build_type>`, so **do not** pass
`--output-folder` to `conan install` — the layout function controls all paths.

### Dockerfile (multi-stage, BuildKit)

The `Dockerfile` uses a BuildKit frontend (`# syntax=docker/dockerfile:1`). Three categories of cache mount are applied to the builder stage:

| `--mount` target | Scope | Effect |
|---|---|---|
| `/var/cache/apt`, `/var/lib/apt` | `apt-get install` | Keeps the apt index off the image layer |
| `/root/.cache/pip` | `pip install conan` | Skips re-downloading the Conan wheel |
| `/root/.conan2/p` | `conan install`, `cmake --build`, `ctest` | Conan binary package store persists across rebuilds |

After building `microservice_app` and `microservice_tests`, the builder stage runs `ctest --test-dir build/Release -VV --progress`. A non-zero exit aborts the Docker build before Stage 2 is reached — the final runtime image is only produced when all unit tests pass.

---

## Reproducing each pipeline stage locally

### Prerequisites

```bash
pip3 install "conan>=2,<3"
```

### 1. Release (GCC)

```bash
conan profile detect --force
cp profiles/gcc ~/.conan2/profiles/default

conan install . --build=missing \
  --profile:host=default --profile:build=default \
  -s:h build_type=Release -s:b build_type=Release

cmake --preset release
cmake --build --preset release --parallel
ctest --test-dir build/Release -VV
```

### 2. Release (Clang 16)

```bash
cp profiles/clang ~/.conan2/profiles/default
# then same conan install / cmake steps as above
```

### 3. Sanitizers (ASAN example)

```bash
cp profiles/clang ~/.conan2/profiles/default

conan install . --build=missing \
  --profile:host=default --profile:build=default \
  -s:h build_type=RelWithDebInfo -s:b build_type=RelWithDebInfo

cmake --preset asan
cmake --build --preset asan --parallel
ctest --test-dir build/ASAN -VV
```

Substitute `tsan` or `ubsan` for the other sanitizers.

### 4. Valgrind

```bash
cp profiles/gcc ~/.conan2/profiles/default

conan install . --build=missing \
  --profile:host=default --profile:build=default \
  -s:h build_type=Debug -s:b build_type=Debug

cmake --preset debug
cmake --build --preset debug --target microservice_tests --parallel

valgrind --tool=memcheck --leak-check=full --show-reachable=yes \
  --num-callers=40 --track-origins=yes --error-exitcode=1 \
  ./build/Debug/tests/microservice_tests
```

### 5. Coverage

```bash
cp profiles/gcc ~/.conan2/profiles/default

conan install . --build=missing \
  --profile:host=default --profile:build=default \
  -s:h build_type=Debug -s:b build_type=Debug

cmake --preset coverage
cmake --build --preset coverage --parallel
cmake --build --preset coverage --target cov_data
# Output: build/Coverage/cov.info.cleaned
# View:   genhtml -o build/Coverage/html build/Coverage/cov.info.cleaned
```

### 6. Docker build (with tests)

```bash
DOCKER_BUILDKIT=1 docker build --progress=plain .
```

Tests are executed inside the builder stage via `ctest`; a test failure aborts the build before Stage 2 (the runtime image) is reached.

---

## MSAN note (CA-04)

The MSAN preset is checked in (`CMakePresets.json`) for local experimentation
but is **not** run in CI. Ubuntu's system `libstdc++` is not memory-instrumented,
which produces false positives from every standard-library allocation. Correctness
is gated on ASAN + Valgrind instead. Reintroduce an `msan` CI job when a
pre-built instrumented libc++ Docker layer is available (see DESIGN.md §12).

---

## Conan cache notes

- Cached path: `~/.conan2/p` (binary package store only — volatile lock files
  in `~/.conan2/` are excluded to prevent cache corruption).
- Cache key: `conan-<os>-<compiler>-<sha256(conanfile.py)>` — invalidates on
  any dependency change.
- `--build=missing` fetches pre-built binaries where available and only compiles
  packages that have no matching binary in the Conan Center cache.
