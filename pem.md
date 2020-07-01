# Проверка самоподписанного сертификата сервера

С целью идентификации общей части кода проверки самоподписанного сертификата сервера, сравнил соответствующую реализацию в проложениях с использование Curllib, Boost.Beast и Android.

Подготовка к проверке в Curllib:

```cpp
curl_easy_setopt(ch, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
rv = curl_easy_perform(ch);
```

Реализация функции sslctx_function:

```cpp
static CURLcode sslctx_function(CURL* curl, void* sslctx, void* parm)
{
    CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

    static const char mypem[] =
        "-----BEGIN CERTIFICATE-----\n"
		...
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
```

