FROM debian:latest
RUN apt-get update && apt-get install gcc build-essential git wget curl -y

WORKDIR /apps

RUN wget https://www.openssl.org/source/openssl-1.1.1i.tar.gz && tar -xzvf openssl-1.1.1i.tar.gz

RUN cd openssl-1.1.1i && ./config  enable-ssl-trace && make && make install

ENV LD_LIBRARY_PATH /usr/local/lib/
ENTRYPOINT ["openssl"]

