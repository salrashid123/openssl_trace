### OpenSSL docker with TLS trace enabled (`enable-ssl-trace`)


Simple docker container with openssl `1.1.1i` which has TLS trace flags enabled.

You can use this to view the low-level TLS traffic between a client and server

to stop the containers, run `docker rm -f client server`

#### With server TLS

```bash
# server
docker run \
  --name server \
  -p 8081:8081 \
  --net=host \
  -v `pwd`/html:/apps/ \
  -v `pwd`/certs:/certs \
  -t docker.io/salrashid123/openssl s_server   \
      -cert /certs/http_server.crt \
      -key /certs/http_server.key \
      -port 8081 \
      -CAfile /certs/tls-ca-chain.pem \
      -tlsextdebug \
      -tls1_3  \
      -trace \
      -WWW

# client
docker run \
  --name client \
  --net=host \
  -v `pwd`/certs/:/certs \
  -t docker.io/salrashid123/openssl s_client \
       -connect localhost:8081 \
       -servername http.domain.com \
       -CAfile /certs/tls-ca-chain.pem \
       -tls1_3 \
       -tlsextdebug \
       -trace

# curl
curl -vvvvv \
  -H "host: http.domain.com" \
  --resolve  http.domain.com:8081:127.0.0.1  \
  --cacert certs/tls-ca-chain.pem \
  https://http.domain.com:8081/index.html
```


#### With mTLS

```bash
#server
docker run \
  --name server \
  -p 8081:8081 \
  --net=host \
  -v `pwd`/html:/apps/ \
  -v `pwd`/certs:/certs \
  -t docker.io/salrashid123/openssl s_server \
       -cert /certs/http_server.crt \
       -key /certs/http_server.key \
       -port 8081 \
       -CAfile /certs/tls-ca-chain.pem \
       -Verify 5 \
       -tlsextdebug \
       -tls1_3  \
       -trace  \
       -WWW

# client
docker run  \
  --name client \
  --net=host \
  -v `pwd`/certs/:/certs \
  -t docker.io/salrashid123/openssl s_client \
       -connect localhost:8081 \
       -servername http.domain.com \
       -CAfile /certs/tls-ca-chain.pem \
       -cert /certs/client.crt \
       -key /certs/client.key \
       -tls1_3 \
       -tlsextdebug \
       --verify 5 \
       -trace

# curl
curl -vvvvv \
  -H "host: http.domain.com" \
  --resolve  http.domain.com:8081:127.0.0.1 \
  --cert certs/client.crt \
  --key certs/client.key \
  --cacert certs/tls-ca-chain.pem \
  https://http.domain.com:8081/index.html
```


#### With OCSP Stapling:

```bash
# server
docker run \
  --name server \
  -p 8081:8081 \
  --net=host -v `pwd`/html:/apps/ \
  -v `pwd`/certs:/certs \
  -t docker.io/salrashid123/openssl s_server \
       -status_file /certs/http_server_ocsp_resp_valid.bin \
       -cert /certs/http_server.crt \
       -key /certs/http_server.key \
       -port 8081 \
       -CAfile /certs/tls-ca-chain.pem \
       -Verify 5 \
       -tlsextdebug \
       -tls1_3 \
       -status \
       -status_verbose \
       -trace \
       -extended_crl \
       -WWW

# client
docker run \
 --name client \
 --net=host \
 -v `pwd`/certs/:/certs \
 -t docker.io/salrashid123/openssl s_client \
      -connect localhost:8081 \
      -servername http.domain.com \
      -CAfile /certs/tls-ca-chain.pem \
      -cert /certs/client.crt \
      -key /certs/client.key \
      -tls1_3 \
      -tlsextdebug \
      -status \
      --verify 5 \
      -trace

# curl
curl -vvvvv \
  -H "host: http.domain.com" \
  --resolve  http.domain.com:8081:127.0.0.1 \
  --cert certs/client.crt \
  --key certs/client.key \
  --cacert certs/tls-ca-chain.pem \
  --cert-status \
  https://http.domain.com:8081/index.html

```


Note, `http_server_ocsp_resp_valid.bin` is a valid OCSP response while `http_server_ocsp_resp_revoked.bin` is an expired one.


To see the details of an OCSP response, run
```bash
docker run \
 -v `pwd`/certs:/certs \
 docker.io/salrashid123/openssl ocsp \
   -CA /certs/tls-ca-ocsp-chain.pem \
   -CAfile /certs/tls-ca-ocsp-chain.pem \
   -respin /certs/http_server_ocsp_resp_valid.bin \
   -text
```


if you want to generate and use your own CA, see [Create Root CA Key and cert](https://github.com/salrashid123/ca_scratchpad)