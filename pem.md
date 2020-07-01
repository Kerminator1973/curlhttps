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

В boost.beast мы загружаем сертификат в boost::asio::ssl::context:

```cpp
m_Context->load_verify_file(m_CertificateName);
```

Проверка сертификата осуществляется через callback:

boost::asio::connect(m_SslSocket->next_layer(), m_Resolver.resolve(m_Host, m_Port));

```cpp
// Perform SSL handshake and verify the remote host's certificate
m_SslSocket->set_verify_mode(boost::asio::ssl::verify_peer);
m_SslSocket->set_verify_callback([](auto&& preverified, auto&& ctx) {
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	return preverified;
});
```

Использовать корневые сертификаты Windows можно так:

```cpp
void HttpsTransport::AddWindowsRootCerts()
{
	HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");
	if (hStore != NULL) {

		X509_STORE* store = X509_STORE_new();
		PCCERT_CONTEXT pContext = NULL;
		while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
			X509* x509 = d2i_X509(NULL,
				(const unsigned char**)&pContext->pbCertEncoded,
				pContext->cbCertEncoded);
			if (x509 != NULL) {
				X509_STORE_add_cert(store, x509);
				X509_free(x509);
			}
		}

		CertFreeCertificateContext(pContext);
		CertCloseStore(hStore, 0);

		SSL_CTX_set_cert_store(m_Context->native_handle(), store);
	}
}
```

Вот такой код был использован в проекте GrandCarWash:

```java
public static void changeApiBaseUrl(Context context, String newApiBaseUrl) {
	apiBaseUrl = newApiBaseUrl;

	// Ниже идёт код, обеспечивающий импорт сертификата для проверки сервера
	// из ресурсов приложения (см. подкаталог "raw", файл "server_public.crt").
	// Статьи по установке сертификата сервера:
	//      https://marthinmhpakpahan.wordpress.com/2016/10/26/implement-ssl-android-retrofit/
	//      https://gist.github.com/erickok/7692592

	// Следующий далее код, содержит несколько обработчиков исключений, что
	// оставлено для демонстрационных целей

	// Используем фабрику сертификатов явно указывая, что мы будем использовать
	// X.509 — стандарт ITU-T для инфраструктуры открытого ключа
	CertificateFactory cf = null;
	try {
		cf = CertificateFactory.getInstance("X.509");
	} catch (CertificateException e) {
		e.printStackTrace();
	}

	// Загружаем сертификаты (CAs) используя InputStream. Сертификат
	// хранится в подкаталоге "raw", в файле "server_public.crt".
	// Генерируем объект Certificate, по стандарту X.509
	InputStream cert = context.getResources().openRawResource(R.raw.server_public);
	Certificate ca=null;
	try {
		ca = cf.generateCertificate(cert);
	} catch (CertificateException e) {
		e.printStackTrace();
	} finally {
		try {
			cert.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	// Создаём хранилище ключей (KeyStore) в котором мы будем хранить
	// доверенные сертификаты (trusted CAs)
	String keyStoreType = KeyStore.getDefaultType();
	KeyStore keyStore = null;
	try {
		keyStore = KeyStore.getInstance(keyStoreType);
	} catch (KeyStoreException e) {
		e.printStackTrace();
	}

	try {
		keyStore.load(null, null);
	} catch (IOException e) {
		e.printStackTrace();
	} catch (NoSuchAlgorithmException e) {
		e.printStackTrace();
	} catch (CertificateException e) {
		e.printStackTrace();
	}

	try {
		keyStore.setCertificateEntry("ca", ca);
	} catch (KeyStoreException e) {
		e.printStackTrace();
	}

	// Создаём компонент TrustManager, который будет доверять сертификатам
	// из созданного нами хранилища сертификатов
	String tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm();
	TrustManagerFactory tmf = null;
	try {
		tmf = TrustManagerFactory.getInstance(tmfAlgorithm);
	} catch (NoSuchAlgorithmException e) {
		e.printStackTrace();
	}
	try {
		tmf.init(keyStore);
	} catch (KeyStoreException e) {
		e.printStackTrace();
	}

	// Создаём фабрику SSL/TLS (SSLSocketFactory), которая использует
	// компонент TrustManager
	SSLContext sslContext = null;
	try {
		sslContext = SSLContext.getInstance("TLS");
	} catch (NoSuchAlgorithmException e) {
		e.printStackTrace();
	}
	try {
		sslContext.init(null, tmf.getTrustManagers(), null);
	} catch (KeyManagementException e) {
		e.printStackTrace();
	}

	// Создаём сервис Retrofit для подключения к нашему WebAPI
	builder = new Retrofit.Builder()
			.client(httpClient.sslSocketFactory(
						sslContext.getSocketFactory(),
						(X509TrustManager) tmf.getTrustManagers()[0])
					.build())
			.addConverterFactory(GsonConverterFactory.create())
			.baseUrl(apiBaseUrl);
}
```

## Общая информация по X.509

**X.509** — стандарт Public key infrastructure (PKI). X.509 определяет стандартные форматы данных и процедуры распределения открытых ключей с помощью соответствующих сертификатов с цифровыми подписями. Эти сертификаты предоставляются удостоверяющими центрами (англ. Certificate Authority).

**.CER** — сертификат, или набор сертификатов, закодированных по стандарту CER.
**.DER** — сертификат, закодированный по стандарту DER.
**.PEM** — PEM-сертификат, закодированный по стандарту DER и использующий **Base64** и помещенный между «----- BEGIN CERTIFICATE -----» и «----- END CERTIFICATE -----».
