### OpenSSL 3.0.0 docker with TLS trace enabled (`enable-ssl-trace`) and FIPS (`enable-fips`)


Simple docker container with the brand new [openssl 3.0.0](https://wiki.openssl.org/index.php/OpenSSL_3.0) which has TLS trace and [FIPS 140-2 enabled by default](https://wiki.openssl.org/index.php/OpenSSL_3.0#Completing_the_installation_of_the_FIPS_Module)

You can use this to view the low-level TLS traffic between a client and server and use openssl with FIPS provider.

- `docker.io/salrashid123/openssl`
- `docker.io/salrashid123/openssl:fips`

>> NOTE: to stop the containers, run `docker rm -f client server`

### FIPS or NO FIPS

- openssl `3.0.0` (COMPAT)


```bash
$ docker run  docker.io/salrashid123/openssl version
    OpenSSL 3.0.0 7 sep 2021 (Library: OpenSSL 3.0.0 7 sep 2021)
```

- openssl `3.0.0` (FIPS)

```bash
$ docker run  docker.io/salrashid123/openssl:fips list -providers
Providers:
  base
    name: OpenSSL Base Provider
    version: 3.0.0
    status: active
  fips
    name: OpenSSL FIPS Provider
    version: 3.0.0
    status: active

# since md5 is not supported in FIPS, you'll see an error
$ docker run docker.io/salrashid123/openssl:fips  md5 /etc/hosts
  Error setting digest
  40607CA1FC7E0000:error:0308010C:digital envelope routines:inner_evp_generic_fetch:unsupported:crypto/evp/evp_fetch.c:346:Global default library context, Algorithm (MD5 : 102), Properties ()
  40607CA1FC7E0000:error:03000086:digital envelope routines:evp_md_init_internal:initialization error:crypto/evp/digest.c:234:

$ docker run docker.io/salrashid123/openssl  md5 /etc/hosts
  MD5(/etc/hosts)= 5a0ff51aeb7b008b497b1ae52adce747
  
```

The [default provider](https://wiki.openssl.org/index.php/OpenSSL_3.0#Providers) is set active in the fips Dockerfile

For usage for openssl3, see [documentation](https://www.openssl.org/docs/man3.0/)

#### With server TLS

```bash
# server
docker run \
  --name server \
  -p 8081:8081 \
  --net=host \
  -v `pwd`/html:/apps/ \
  -v `pwd`/certs:/certs \
  -ti docker.io/salrashid123/openssl s_server   \
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
  -ti docker.io/salrashid123/openssl s_client \
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
  -ti docker.io/salrashid123/openssl s_server \
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
  -ti docker.io/salrashid123/openssl s_client \
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
  -ti docker.io/salrashid123/openssl s_server \
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
 -ti docker.io/salrashid123/openssl s_client \
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


#### TLS Decryption with wireshark

If you want to decrypt the actual TLS traffic using [wireshark](https://www.wireshark.org/), set an env-var for curl/openssl pointing to the keys `SSLKEYLOGFILE` 

Start Wireshark, under `Preferences->Protocols->TLS` set "(Pre)-Master-Secret log filename" to `/tmp/keylog.log`

Begin capture on `lo` (localhost) interface and filter on `tcp.port==8081`


Then run 

```bash
# server
docker run \
  --name server \
  -p 8081:8081 \
  --net=host \
  -v `pwd`/html:/apps/ \
  -v `pwd`/certs:/certs \
  -ti docker.io/salrashid123/openssl s_server   \
      -cert /certs/http_server.crt \
      -key /certs/http_server.key \
      -port 8081 \
      -CAfile /certs/tls-ca-chain.pem \
      -tlsextdebug \
      -tls1_3  \
      -trace \
      -WWW

# curl
export SSLKEYLOGFILE=/tmp/keylog.log
curl -vvvvv \
  -H "host: http.domain.com" \
  --resolve  http.domain.com:8081:127.0.0.1 \
  --cert certs/client.crt \
  --key certs/client.key \
  --cacert certs/tls-ca-chain.pem \
  https://http.domain.com:8081/index.html
```

You should see the decrypted traffic (in this case the HTTPS response from the server, `ok`)

![images/tls_decryption.png](images/tls_decryption.png)

I've left a sample keylog file and associated tls capture (just load `tls.pcapng` in wireshark and then specify the path to `keylog.log` as the master-secret log file)


---


### FIPS

- [https://www.openssl.org/docs/manmaster/man7/fips_module.html](https://www.openssl.org/docs/manmaster/man7/fips_module.html)

```bash
$ docker run  -t docker.io/salrashid123/openssl:fips ciphers --provider fips

TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:DHE-RSA-AES256-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:DHE-RSA-AES128-SHA:RSA-PSK-AES256-GCM-SHA384:DHE-PSK-AES256-GCM-SHA384:AES256-GCM-SHA384:PSK-AES256-GCM-SHA384:RSA-PSK-AES128-GCM-SHA256:DHE-PSK-AES128-GCM-SHA256:AES128-GCM-SHA256:PSK-AES128-GCM-SHA256:AES256-SHA256:AES128-SHA256:ECDHE-PSK-AES256-CBC-SHA384:ECDHE-PSK-AES256-CBC-SHA:SRP-RSA-AES-256-CBC-SHA:SRP-AES-256-CBC-SHA:RSA-PSK-AES256-CBC-SHA384:DHE-PSK-AES256-CBC-SHA384:RSA-PSK-AES256-CBC-SHA:DHE-PSK-AES256-CBC-SHA:AES256-SHA:PSK-AES256-CBC-SHA384:PSK-AES256-CBC-SHA:ECDHE-PSK-AES128-CBC-SHA256:ECDHE-PSK-AES128-CBC-SHA:SRP-RSA-AES-128-CBC-SHA:SRP-AES-128-CBC-SHA:RSA-PSK-AES128-CBC-SHA256:DHE-PSK-AES128-CBC-SHA256:RSA-PSK-AES128-CBC-SHA:DHE-PSK-AES128-CBC-SHA:AES128-SHA:PSK-AES128-CBC-SHA256:PSK-AES128-CBC-SHA


docker run   -ti docker.io/salrashid123/openssl:fips s_client \
   -connect www.google.com:443  \
   -servername www.google.com -tlsextdebug -trace
```

Sample c program that confirms FIPS status

```bash
docker run  -v `pwd`/cli:/cli --entrypoint /bin/bash -ti docker.io/salrashid123/openssl:fips 
```
then

```bash
cd /cli
gcc main.c -lcrypto -o main
./main
FIPS provider loaded
Base provider loaded
0679246d6c4216de0daa08e5523fb2674db2b6599c3b72ff946b488a15290b62
```


### Google CA

Included in this repo is the specific set of [Google CAs](https://pki.goog/repository/) (as of 8/2/22)

```bash
docker run   --name client  \
  --net=host \
  -v `pwd`/certs/:/certs \
  -ti docker.io/salrashid123/openssl s_client  \
        -connect storage.googleapis.com:443  \
        -servername storage.googleapis.com   \
        -CAfile  /certs/google_ca.pem   \
        -tls1_3  \
        -tlsextdebug \
        -trace
```