# Код приложений, использующий curl

Множество примеров кода доступно [по ссылке на официальном сайте](https://curl.haxx.se/libcurl/c/example.html). Типовой пример:

```cpp
#include <iostream>
#include "curl/curl.h"

int main(void)
{
	CURL *curl;
	CURLcode result;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com/");   // Set URL
		
		// If you want to set any more options, do it here, before making the request.
		// Perform the request which prints to stdout
		result = curl_easy_perform(curl);
		// Error check
		if (result != CURLE_OK) {
			std::cerr << "Error during curl request: " 
				<< curl_easy_strerror(result) << std::endl;
		}
		
		curl_easy_cleanup(curl);
		
	} else {
		std::cerr << "Error initializing curl." << std::endl;
	}

	curl_global_cleanup();
	return 0;
	}
```

Простейший вариант использования https (без аутентификации host-а и peer-а):

```cpp
curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
result = curl_easy_perform(curl);
```

Для того, чтобы curl проверял сертификат сервера, следует скачать [CA сертификаты Mozilla](https://curl.se/docs/caextract.html)

Проверка имени хоста и сертификата в сертификате осуществляется, если установить следующие флаги:

```cpp
curl_easy_setopt( curl, CURLOPT_CAINFO, "c:/Temp/cacert.pem");
curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 2L );
curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 2L );
```

При  использовании опции CURLOPT_CAINFO загрузка CA сертификатов выполняется из внешнего файла, что не всегда практично для промышленных приложений. Можно хранить CA сертификаты в самом приложении (например, в зашифрованном виде) и загружать их динамически:

``` cpp
char *strpem; // strpem must point to a PEM string
CURL *curl = curl_easy_init();
if (curl) {
    struct curl_blob blob;
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");
    blob.data = strpem;
    blob.len = strlen(strpem);
    blob.flags = CURL_BLOB_COPY;
    curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
    ret = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
```

Использование клиентского сертификата в SSL-соединении есть в [примере simplessl](https://curl.haxx.se/libcurl/c/simplessl.html). См.:

```cpp
/* set the cert for client authentication */ 
curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);
```

Выполнение аутентификации сервера с использованием самоподписанного сертификата, хранимого на клиенте можно увидеть в [примере cacertinmem](https://curl.haxx.se/libcurl/c/cacertinmem.html). См.:

```cpp
/* set the file with the certs vaildating the server */ 
curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);
```

## Получить данные от сервера. Пример

Сначала требуется разработать функцию накопления возвращаемых результатов:

``` cpp
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
	size_t newLength = size*nmemb;
	try
	{
		s->append((char*)contents, newLength);
	}
	catch (std::bad_alloc &e)
	{
		//handle memory problem
		return 0;
	}
	return newLength;
}
```

Далее эту функцию можно применить, указав в качестве накопителя информации std::string:

``` cpp
std::string s;

CURL *curl;
CURLcode result;

curl_global_init(CURL_GLOBAL_DEFAULT);
curl = curl_easy_init();
if (curl) {
	curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:3000/");   // Set URL

	// If you want to set any more options, do it here, before making the request.
	// Perform the request which prints to stdout

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

	result = curl_easy_perform(curl);
	...
	curl_easy_cleanup(curl);
```

Вывод сообщения в MFC-приложении:

``` cpp
::AfxMessageBox(s.c_str(), MB_OK);
```
