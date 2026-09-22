FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    autoconf automake libtool \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /app
COPY vcpkg.json .
RUN /opt/vcpkg/vcpkg install --triplet x64-linux

COPY . .
RUN cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release -G Ninja
RUN cmake --build build --target anujamart

FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ca-certificates libssl3 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/anujamart /app/anujamart
COPY --from=build /app/public /app/public
COPY --from=build /app/db /app/db

CMD ["/app/anujamart"]
