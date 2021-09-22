#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/provider.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/err.h>
#include <openssl/bio.h>

typedef unsigned char byte;

void handleErrors(void)
{
    unsigned long errCode;

    printf("An error occurred\n");
    while(errCode = ERR_get_error())
    {
        char *err = ERR_error_string(errCode, NULL);
        printf("%s\n", err);
    }
    abort();
}

int main(void)
{
    OSSL_PROVIDER *fips;
    OSSL_PROVIDER *base;
   
    fips = OSSL_PROVIDER_load(NULL, "fips");
    if (fips == NULL) {
        printf("Failed to load FIPS provider\n");
        exit(EXIT_FAILURE);
    }
    printf("FIPS provider loaded\n");

    base = OSSL_PROVIDER_load(NULL, "base");
    if (base == NULL) {
        OSSL_PROVIDER_unload(fips);
        printf("Failed to load base provider\n");
        exit(EXIT_FAILURE);
    }
    printf("Base provider loaded\n");

    /* Rest of application */

	EVP_MD_CTX *mdctx;
    const int message_len = 30;
    byte digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    int i;
    byte* message = (byte *)malloc(message_len);

    for (i=0; i<message_len; i++) message[i] = 0;

    if((mdctx = EVP_MD_CTX_new()) == NULL)
		handleErrors();

	if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
		handleErrors();

	if(1 != EVP_DigestUpdate(mdctx, message, message_len))
		handleErrors();

	if((*digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
		handleErrors();

	if(1 != EVP_DigestFinal_ex(mdctx, digest, &digest_len))
		handleErrors();


    for (i=0; i<digest_len; i++)
	printf("%02x", digest[i]);
    putchar('\n');


    OSSL_PROVIDER_unload(base);
    OSSL_PROVIDER_unload(fips);
    exit(EXIT_SUCCESS);
   }

