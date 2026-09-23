FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_MAX_CONCURRENCY=1
ENV CMAKE_BUILD_PARALLEL_LEVEL=1

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git curl zip unzip tar pkg-config python3 bison flex \
    autoconf automake libtool \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /app
COPY vcpkg.json .
RUN mkdir -p /opt/triplets && \
    cp /opt/vcpkg/triplets/x64-linux.cmake /opt/triplets/x64-linux-release.cmake && \
    echo "set(VCPKG_BUILD_TYPE release)" >> /opt/triplets/x64-linux-release.cmake
RUN /opt/vcpkg/vcpkg install --triplet x64-linux-release --overlay-triplets=/opt/triplets || (find /opt/vcpkg/buildtrees/libsodium -iname "*-out.log" -exec cat {} \; || true; exit 1)

COPY . .
RUN cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux-release \
    -DVCPKG_OVERLAY_TRIPLETS=/opt/triplets \
    -DVCPKG_MANIFEST_MODE=OFF \
    -DCMAKE_PREFIX_PATH=/opt/vcpkg/installed/x64-linux-release \
    -DCMAKE_BUILD_TYPE=Release -G Ninja
RUN cmake --build build --target anujamart

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ca-certificates libssl3 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/anujamart /app/anujamart
COPY --from=build /app/public /app/public
COPY --from=build /app/db /app/db

CMD ["/app/anujamart"]
