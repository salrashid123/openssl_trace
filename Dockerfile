FROM debian:latest
RUN apt-get update && apt-get install gcc build-essential git wget curl vim -y

WORKDIR /apps

RUN wget https://www.openssl.org/source/openssl-3.0.0.tar.gz && tar -xzvf openssl-3.0.0.tar.gz

RUN cd openssl-3.0.0 && ./config enable-ssl-trace && make && make install
ENV LD_LIBRARY_PATH /usr/local/lib/:/usr/local/lib64/

ENTRYPOINT ["openssl"]