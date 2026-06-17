FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    g++ \
    git \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build
WORKDIR /app/build
RUN ctest --output-on-failure
FROM ubuntu:22.04 AS final
RUN apt-get update && apt-get install -y --no-install-recommends \
    libc6 \
    libstdc++6 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/build/network_simulator .
COPY --from=builder /app/build/tests/unit_tests .
COPY --from=builder /app/build/tests/scenario1_simple_transfer .
COPY --from=builder /app/build/tests/scenario2_collision .
COPY --from=builder /app/build/tests/scenario3_routing .
COPY --from=builder /app/build/tests/scenario4_heavy_load .
COPY --from=builder /app/build/tests/scenario5_mixed_traffic .
COPY --from=builder /app/build/tests/scenario6_network_failure .
COPY --from=builder /app/docker-entrypoint.sh .
RUN chmod +x docker-entrypoint.sh

ENTRYPOINT ["./docker-entrypoint.sh"]
CMD []