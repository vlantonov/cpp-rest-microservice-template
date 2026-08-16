# ---------------------------------------------------------------------------
# Stage 1 — builder
# ---------------------------------------------------------------------------
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake \
        ninja-build \
        gcc \
        g++ \
        python3-pip \
        python3-venv \
        git \
        curl \
        pkg-config \
        libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Conan 2 into a virtual env to avoid PEP 668 issues
RUN python3 -m venv /opt/conan-venv \
 && /opt/conan-venv/bin/pip install --no-cache-dir "conan>=2,<3"

ENV PATH="/opt/conan-venv/bin:${PATH}"

# Initialise Conan default profile
RUN conan profile detect --force

WORKDIR /src
COPY . .

# Use the project GCC profile as the Conan default
RUN cp profiles/gcc ~/.conan2/profiles/default

# Install Conan dependencies (Release build)
RUN conan install . \
        --build=missing \
        --profile:host=default \
        --profile:build=default \
        -s:h build_type=Release \
        -s:b build_type=Release

# Configure and build
RUN cmake --preset release \
 && cmake --build --preset release --parallel "$(nproc)" --target microservice_app

# Strip the binary to reduce image size
RUN find build/Release -name microservice_app -type f -exec strip {} +

# ---------------------------------------------------------------------------
# Stage 2 — runtime
# ---------------------------------------------------------------------------
FROM ubuntu:24.04

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
