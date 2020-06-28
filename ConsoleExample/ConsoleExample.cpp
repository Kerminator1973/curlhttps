/*

// Adapted from https://curl.haxx.se/libcurl/c/https.html

#include <iostream>
#include "curl/curl.h"

int main()
{
    CURL* curl;
    CURLcode result;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {

        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");
        // If you want to set any more options, do it here, before making the request.

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // Perform the request which prints to stdout
        result = curl_easy_perform(curl);

        // Error check
        if (result != CURLE_OK) {
            std::cerr << "Error during curl request: "
                << curl_easy_strerror(result) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Error initializing curl." << std::endl;
    }

    curl_global_cleanup();

    return 0;
}

*/


#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <stdio.h>

static size_t writefunction(void* ptr, size_t size, size_t nmemb, void* stream)
{
    fwrite(ptr, size, nmemb, (FILE*)stream);
    return (nmemb * size);
}

static CURLcode sslctx_function(CURL* curl, void* sslctx, void* parm)
{
    CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

    /** This example uses two (fake) certificates **/
    static const char mypem[] =

        "-----BEGIN CERTIFICATE-----\n"
        "MIIDtDCCApygAwIBAgIJAKhnVsbAm4hTMA0GCSqGSIb3DQEBCwUAMIGFMQswCQYD\n"
        "VQQGEwJSVTELMAkGA1UECAwCUlUxDzANBgNVBAcMBk1vc2NvdzENMAsGA1UECgwE\n"
        "RE9SUzEWMBQGA1UECwwNSVQgRGVwYXJ0bWVudDEdMBsGCSqGSIb3DQEJARYOdGVz\n"
        "dEBsb2NhbGhvc3QxEjAQBgNVBAMMCWxvY2FsaG9zdDAeFw0yMDA2MjgxMjI2MTla\n"
        "Fw0zMDAzMjgxMjI2MTlaMIGFMQswCQYDVQQGEwJSVTELMAkGA1UECAwCUlUxDzAN\n"
        "BgNVBAcMBk1vc2NvdzENMAsGA1UECgwERE9SUzEWMBQGA1UECwwNSVQgRGVwYXJ0\n"
        "bWVudDEdMBsGCSqGSIb3DQEJARYOdGVzdEBsb2NhbGhvc3QxEjAQBgNVBAMMCWxv\n"
        "Y2FsaG9zdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMEYS9I5KlAR\n"
        "GtUcQyxwQmTJGq3KknXI/qYRur0vM87TzYnBJgcr2lHZxNQ9kvKZR2dhWQbtZdEx\n"
        "UwcPDWj1F/+hxmSTHrHLlUhgeFau9S9YxTy83JpEuxucdiz6HXIS1pep64tOdqVs\n"
        "r7eHAZ47PXyd2ktBEMi5cuzG7ttMO5JlXhHawnbVQhC58u+3J4qEvYnQpOoCWxpn\n"
        "0SOoN8/dxDm13sLQSVvS8YNwRq6qjDhtkqw9OlEx8cvi+rDF6rUq0svnwi71HywQ\n"
        "/56vJ34/fwhSuPkPu9n7GmQuO1FvVaMfOvyqFui7FZsyw02/5ZGY1WjkrtOmlswj\n"
        "hrTUP5yMcJUCAwEAAaMlMCMwIQYDVR0RBBowGIILKi5sb2NhbGhvc3SCCWxvY2Fs\n"
        "aG9zdDANBgkqhkiG9w0BAQsFAAOCAQEAa4OzfMKv4DQaarWIHr9DR69VSPrl6B/6\n"
        "N6IwuHnfhmJN+nQsTiU3wzaO/xeb2sfV/gBESldHkycBHvXwmYEiaiuEQu/CveNd\n"
        "NCchIZ3Ek103I7GR58JI2Bo/1gO4OerhhYsJe5H+D0L7KNFrakwVMaHg1rY+JVKW\n"
        "InANaRHZcAmqNegODh8KNoaCdJ729yBvDoYJX5NhWWcSnhVr919P1wKrqRnCGOmP\n"
        "lXOiujIuNOtzztsrMUzzZf/4kR3cFrsyT1BRza34AY7jQMNlh3wBnDN9OKS4Daxx\n"
        "eHWU18s1hT2kxbhhKSHN4I6C8dnHnRJLtxPX2wWC7KN6Tcbc3FK/DA==\n"
        "-----END CERTIFICATE-----\n";

    BIO* cbio = BIO_new_mem_buf(mypem, sizeof(mypem));
    X509_STORE* cts = SSL_CTX_get_cert_store((SSL_CTX*)sslctx);
    int i;
    STACK_OF(X509_INFO)* inf;
    (void)curl;
    (void)parm;

    if (!cts || !cbio) {
        return rv;
    }

    inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

    if (!inf) {
        BIO_free(cbio);
        return rv;
    }

    for (i = 0; i < sk_X509_INFO_num(inf); i++) {
        X509_INFO* itmp = sk_X509_INFO_value(inf, i);
        if (itmp->x509) {
            X509_STORE_add_cert(cts, itmp->x509);
        }
        if (itmp->crl) {
            X509_STORE_add_crl(cts, itmp->crl);
        }
    }

    sk_X509_INFO_pop_free(inf, X509_INFO_free);
    BIO_free(cbio);

    rv = CURLE_OK;
    return rv;
}

int main(void)
{
    CURL* ch;
    CURLcode rv;

    curl_global_init(CURL_GLOBAL_ALL);
    ch = curl_easy_init();
    curl_easy_setopt(ch, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(ch, CURLOPT_HEADER, 0L);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, *writefunction);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, stdout);
    curl_easy_setopt(ch, CURLOPT_HEADERFUNCTION, *writefunction);
    curl_easy_setopt(ch, CURLOPT_HEADERDATA, stderr);
    curl_easy_setopt(ch, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(ch, CURLOPT_URL, "https://localhost:3000/");

    /* Turn off the default CA locations, otherwise libcurl will load CA
     * certificates from the locations that were detected/specified at
     * build-time
     */
    curl_easy_setopt(ch, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(ch, CURLOPT_CAPATH, NULL);

    /* first try: retrieve page without ca certificates -> should fail
     * unless libcurl was built --with-ca-fallback enabled at build-time
     */
    rv = curl_easy_perform(ch);
    if (rv == CURLE_OK)
        printf("*** transfer succeeded ***\n");
    else
        printf("*** transfer failed ***\n");

    /* use a fresh connection (optional)
     * this option seriously impacts performance of multiple transfers but
     * it is necessary order to demonstrate this example. recall that the
     * ssl ctx callback is only called _before_ an SSL connection is
     * established, therefore it will not affect existing verified SSL
     * connections already in the connection cache associated with this
     * handle. normally you would set the ssl ctx function before making
     * any transfers, and not use this option.
     */
    curl_easy_setopt(ch, CURLOPT_FRESH_CONNECT, 1L);

    /* second try: retrieve page using cacerts' certificate -> will succeed
     * load the certificate by installing a function doing the necessary
     * "modifications" to the SSL CONTEXT just before link init
     */
    curl_easy_setopt(ch, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
    rv = curl_easy_perform(ch);
    if (rv == CURLE_OK)
        printf("*** transfer succeeded ***\n");
    else
        printf("*** transfer failed ***\n");

    curl_easy_cleanup(ch);
    curl_global_cleanup();
    return rv;
}