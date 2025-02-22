FROM ubuntu:latest 

# Install dependencies (add 'tcl' and 'wget')
RUN apt-get update && apt-get install -y \
    g++ \
    git \
    cmake \
    make \
    libssl-dev \
    pkg-config \
    tcl \
    wget

# Compile SRT with OpenSSL explicitly linked
RUN git clone https://github.com/Haivision/srt.git && \
    cd srt && \
    ./configure --prefix=/usr \
        --enable-static=no \
        --enable-shared=yes \
        --disable-c++11 \
        --disable-ipv6 \
        --with-openssl-include=/usr/include/openssl \
        --with-openssl-lib=/usr/lib/x86_64-linux-gnu && \
    make -j$(nproc) && \
    make install

# Build your server with full linker flags
COPY server.cpp .
RUN g++ -o /usr/local/bin/srt_server server.cpp \
    -I/usr/include/srt \
    -lsrt \
    -lssl -lcrypto \
    -lpthread -ldl