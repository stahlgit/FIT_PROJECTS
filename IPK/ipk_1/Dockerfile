FROM ubuntu:latest 

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    iputils-ping \
    iputils-tracepath \
    libpcap-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

CMD ["./ipk-l2l3-scan", "-i", "eth0", "-s", "172.18.0.0/28", "-s", "172.18.0.0/30", "-s", "fd00::/125" ]