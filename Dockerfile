# syntax=docker/dockerfile:1
# ---------------------------------------------------------------------------
# Stage 1 — builder
# ---------------------------------------------------------------------------
FROM ubuntu:26.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Cache mounts keep apt indexes off the image layer; no rm -rf needed here.
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
        cmake \
        ninja-build \
        gcc \
        g++ \
        python3-pip \
        python3-venv \
        git \
        curl \
        pkg-config \
        libssl-dev

# venv avoids PEP 668 conflicts; pip cache mount skips re-downloading the wheel.
RUN --mount=type=cache,target=/root/.cache/pip \
    python3 -m venv /opt/conan-venv \
 && /opt/conan-venv/bin/pip install "conan>=2,<3"

ENV PATH="/opt/conan-venv/bin:${PATH}"

RUN conan profile detect --force

WORKDIR /src
COPY . .

# Use the project GCC profile as the Conan default.
RUN cp profiles/gcc ~/.conan2/profiles/default

# Cache mount keeps the binary package store across rebuilds; generated
# toolchain files land under /src/build and are stored in the image layer.
RUN --mount=type=cache,target=/root/.conan2/p \
    conan install . \
        --build=missing \
        --profile:host=default \
        --profile:build=default \
        -s:h build_type=Release \
        -s:b build_type=Release

# Same cache mount required here: CMake resolves headers and libs through it.
RUN --mount=type=cache,target=/root/.conan2/p \
    cmake --preset release \
 && cmake --build --preset release --parallel "$(nproc)" --target microservice_app \
 && cmake --build --preset release --parallel "$(nproc)" --target microservice_tests

# Failing tests abort the build before Stage 2 is reached.
RUN --mount=type=cache,target=/root/.conan2/p \
    ctest --test-dir build/Release -VV --progress

# Strip the binary to reduce image size.
RUN find build/Release -name microservice_app -type f -exec strip {} +

# ---------------------------------------------------------------------------
# Stage 2 — runtime
# ---------------------------------------------------------------------------
FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libstdc++6 \
        libssl3 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder \
    /src/build/Release/src/application/microservice_app \
    /usr/local/bin/microservice_app

EXPOSE 8080

ENV LOG_LEVEL=info
ENV SERVICE_NAME=cpp-microservice

ENTRYPOINT ["/usr/local/bin/microservice_app"]
