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
