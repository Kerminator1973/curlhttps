# Использование клиентского сертификата

Включение клиентского сертификата в процедуру установки соединения (_handshake_) позволяет обеспечить двух-стороннюю аутентификацию.

Заметим, что под термином **сертификат** подразумевается контейнер, в котором кроме публичного ключа размещаются:

- параметры открытого ключа (какие операции можно выполнять с его помощью)
- информация о владельце
- сведения об удостоверяющем центре
- электронная цифровая подпись

Для поддержки аутентификации клиента необходим private key (.key) клиента и его сертификат (.crt). Часто обе составляющие части объединяют в контейнер (.pem, или .pfx/.p12). Для проверки клиента сервер генерирует случайное значение (Challenge), который клиент подписывает своим private key и возвращает серверу Thumbprint.

Проверку клиентского кода можно осуществлять используя ресурс [badssl](https://badssl.com). В разделе [Client Certificate](https://badssl.com/download/) можно загрузить корректные клиентские сертификаты. Далее клиент может подключиться к ресурсу https://client.badssl.com и предъявить этот сертификат. Если клиентский код написан корректно, то сервер выполнит аутентификацию клиента и вернёт ответ с Http Status Code равным 200 (OK).

До разработки полноценного клиентского кода рекомендуется выполнить проверку соединения используя утилиты [Postman](https://www.postman.com/), или [curl](https://curl.se/).

Клиентский сертификат может быть встроен в приложение, либо размещён в локальном хранилище сертификатов (например, используя утилиту mmc) в раздел "**Personal/Личное**". Если клиентский сертификат есть в локальном хранилище, то при попытке подключиться к серверу, требующий пользовательский сертификат, браузер (Edge, Chrome) выведет на экран список установленных клиентских сертификатов и предложит выбрать актуальный.

## Postman

Для выполнения запроса с аутентификацией клиента необходимо указать клиентский сертификат в Settings -> Certificate. Для подключения к badssl требуется указать файл "badssl.com-client.p12", host - "client.badssl.com", а также пароль "badssl.com". Пример настройки:

![alt text](./PostmanAddClientCertificate.png "Client certificate's settings for Postman")

Запрос к сайту должен выглядеть следующим образом: https://client.badssl.com

При работе в корпоративной сети с прокси, необходимо настроить подключение в Settings -> Proxy.

## Curl

Ещё один вариант проверки схемы без разработки клиентского кода - использовать утилиту Curl. Следует иметь ввиду, что curl может быть собран как с openssl, так и без этой библиотеки. Использование сборки с openssl является критичным.

Команда подключения к client.badssl.com из корпоративной сети через прокси может выглядеть так:

``` shell
curl -v -i -k --cert badssl.com‐client.pem:badssl.com -x 192.168.100.200:3128 https://client.badssl.com
```

Параметры команды:

По сравнению с текстом статьи, я указал корпоративный прокси. Параметры:

- -E, --cert [используемый сертификат:пароль]
- -v, --verbose необходима подробная информация
- -i, --include выводить информацию о MIME-заголовках
- -k, --insecure по умолчанию, curl проверяет соединение на безопасность. Этот ключ отключает проверку.
- -x использовать прокси-сервер

Ключевая особенность curl состоит в том, что в verbose-режиме осуществляется очень подробное протоколирование процесса подключения к серверу. Пример лога:

``` log
* CONNECT phase completed
* ALPN: offers h2
* ALPN: offers http/1.1
* TLSv1.0 (OUT), TLS header, Certificate Status (22):
* TLSv1.3 (OUT), TLS handshake, Client hello (1):
* TLSv1.2 (IN), TLS header, Certificate Status (22):
* TLSv1.3 (IN), TLS handshake, Server hello (2):
* TLSv1.2 (IN), TLS header, Certificate Status (22):
* TLSv1.2 (IN), TLS handshake, Certificate (11):
* TLSv1.2 (IN), TLS header, Certificate Status (22):
* TLSv1.2 (IN), TLS handshake, Server key exchange (12):
* TLSv1.2 (IN), TLS header, Certificate Status (22):
* TLSv1.2 (IN), TLS handshake, Request CERT (13):
* TLSv1.2 (IN), TLS handshake, Server finished (14):
* TLSv1.2 (OUT), TLS header, Certificate Status (22):
* TLSv1.2 (OUT), TLS handshake, Certificate (11):
* TLSv1.2 (OUT), TLS header, Certificate Status (22):
* TLSv1.2 (OUT), TLS handshake, Client key exchange (16):
* TLSv1.2 (OUT), TLS header, Certificate Status (22):
* TLSv1.2 (OUT), TLS handshake, CERT verify (15):
* TLSv1.2 (OUT), TLS header, Finished (20):
* TLSv1.2 (OUT), TLS change cipher, Change cipher spec (1):
* TLSv1.2 (OUT), TLS header, Certificate Status (22):
* TLSv1.2 (OUT), TLS handshake, Finished (20):
* TLSv1.2 (IN), TLS header, Finished (20):
* TLSv1.2 (IN), TLS header, Certificate Status (22):
* TLSv1.2 (IN), TLS handshake, Finished (20):
* SSL connection using TLSv1.2 / ECDHE-RSA-AES128-GCM-SHA256
* ALPN: server accepted http/1.1
* Server certificate:
*  subject: CN=*.badssl.com
*  start date: Aug 12 14:57:46 2022 GMT
*  expire date: Nov 10 14:57:45 2022 GMT
*  issuer: C=US; O=Let's Encrypt; CN=R3
*  SSL certificate verify result: unable to get local issuer certificate (20), continuing anyway.
* TLSv1.2 (OUT), TLS header, Supplemental data (23):
> GET / HTTP/1.1
> Host: client.badssl.com
> User-Agent: curl/7.84.0
> Accept: */*
```

## Пример клиентского кода на C\#

Ниже приведён пример приложения на C#, который отправляет клиентский сертификат на сервер badssl.com:

``` csharp
using System.Net;
using System.Security.Cryptography.X509Certificates;

// Для доступа к сайтам в интернет, необходимо указать Proxy
var proxy = new WebProxy
{
    Address = new Uri($"http://192.168.100.200:3128"),
    BypassProxyOnLocal = false,
    UseDefaultCredentials = true,
};

// Указываем, что следует идти через прокси
var handler = new HttpClientHandler()
{
    Proxy = proxy,
};

// Добавляем клиентский сертификат, который был нам предоставлен сайтом badssl.com
var cert = new X509Certificate2("c:/Temp/badssl.com-client.p12", "badssl.com");
handler.ClientCertificates.Add(cert);

var client = new HttpClient(handler);

// Описываем запрос по https
var request = new HttpRequestMessage()
{
    RequestUri = new Uri("https://client.badssl.com"),
    Method = HttpMethod.Get,
};

// Выполняем запрос 
var response = await client.SendAsync(request);
if (response.IsSuccessStatusCode)
{
    var responseContent = response.Content.ReadAsStream();
    Console.WriteLine(new StreamReader(responseContent).ReadToEnd());
}
```

Генерация клиентского сертификата в интранет описана [здесь](./request_cert.md).

### Проверка клиентского сертификата приложением ASP.NET Core 6

Ключевая статья [Configure certificate authentication in ASP.NET Core](https://learn.microsoft.com/en-us/aspnet/core/security/authentication/certauth?view=aspnetcore-6.0)

Для удобства рекомендуется добавить класс-расширение AuthenticationExtension, например, создав файл "AuthenticationExtension.cs":

``` csharp
using Microsoft.AspNetCore.Authentication.Certificate;
using System.Security.Cryptography.X509Certificates;

namespace RUFServerObjs
{
    public static class AuthenticationExtension
    {
        public static void ConfigureAuthetication(this IServiceCollection services)
        {
            services.AddAuthentication(CertificateAuthenticationDefaults.AuthenticationScheme)
                .AddCertificate(options =>
                {
                    options.RevocationMode = X509RevocationMode.NoCheck;
                    options.AllowedCertificateTypes = CertificateTypes.All;

                    options.ChainTrustValidationMode = X509ChainTrustMode.System;
                    options.ValidateCertificateUse = false;
                    options.ValidateValidityPeriod = false;

                    options.Events = new CertificateAuthenticationEvents
                    {
                        OnCertificateValidated = context =>
                        {
                            if ((context.ClientCertificate != null) &&
                                (String.Compare(context.ClientCertificate.Subject, "E=isp@dors.com", true) == 0))
                            {
                                context.Success();
                            }
                            else 
                            {
                                context.Fail("invalid subject");
                            }

                            return Task.CompletedTask;
                        },
                        OnAuthenticationFailed = context => // Не обязательно
                        {
                            context.Fail($"Invalid certificate");
                            return Task.CompletedTask;
                        },
                        OnChallenge = context =>            // Не обязательно
                        {
                            // Метод вызывается перед отправкой запроса вызывающей стороне

                            // По умолчанию мы должны вернуть флаг выполнения задачи
                            return Task.CompletedTask;
                        }
                    };
                });

            services.AddAuthorization();
        }
    }
}
```

Приведённый выше код проверяет, что предоставленный клиентский сертификат был выдан владельцу, который обладает почтовым адресом "E=isp@dors.com". Могут быть добавлены и другие проверки, такие как Thumbprint.

При настройке сервисов в приложении (dependency injection) добавляем сервис проверки аутентификации - реализация этого вызова приведена выше по коду:

``` csharp
var builder = WebApplication.CreateBuilder(args);

// Добавляем проверку клиентского сертификата
builder.Services.ConfigureAuthetication();

// Настраиваем Kestrel таким образом, чтобы требовался клиентский сертификат
builder.WebHost.ConfigureKestrel(serverOptions =>
{
    serverOptions.ConfigureHttpsDefaults(o =>
    {
        o.ClientCertificateMode = ClientCertificateMode.RequireCertificate;
    });
});

var app = builder.Build();
```

Далее необходимо активировать механизм аутентификации:

``` csharp
app.UseAuthentication();
app.UseAuthorization();
```

Обработчики запросов, доступ к которым следует предоставить только при предъявлении клиентского сертификата, следует настроить соответствующим образом:

``` csharp
app.MapGet("/{device_id}", (string device_id, HttpContext httpContext, RUFObjsContext dbContext, HttpRequest request) =>
{
    ...
}).RequireAuthorization();
```

### Типовая схема использования клиентского сертификата совместно с браузером

На интранет-ресурсе выполняется генерация пары ключей и пользователю предоставлявляется возможность установить private key в локальное хранилище ключей по ссылке в браузере (ссылка может называться "Install the certificate"). Клиент подтверждает необходимость установки ключей в хранилище и может просмотреть их используя "консоль оснастки" (mmc).

Проверить наличие ключа в хранилище можно запустив "консоль оснастки", добавив в остастку "Сертификаты" и перейдя в ветку дерева: "Сертификаты - текущий пользователь" -> Личное -> Сертификаты

Так же на интранет-ресурсе можно загрузить сертификат с публичным ключом пользователя, который может быть установлен на web-сервере.

Заметим, что сохранённый private key клиентского сертификата может быть зашифрован паролем.

Когда пользователь заходит на сайт выполняющий проверку клиентского сертификата браузер анализирует список клиентских сертификатов текущего пользователя и предлагает выбрать сертификат из списка. Для выбранного сертификата пользователь вводит пароль, private key расшифровывается в памяти браузера и осуществляется процедура установки защищённого соединения с сайтом. В процессе установки соединения сервер присылает браузеру challenge (случайное значение, часто UUID, или GUID), который браузер подписывает своим private key. Сервер проверяет цифровую подпись используя публичный ключ, входящий в состав клиентского сертификата.

Рекомендуется ознакомиться со статьёй [Configuring IIS for Client Certificate Mapping Authentication](https://medium.com/@yildirimabdrhm/configuring-iis-for-client-certificate-mapping-authentication-d7f707506a97) by Abdurrahim Yıldırım.

## Хранилище сертификатов в Windows

Доступ к хранилищу сертификатов Windows возможен через "консоль оснастки" (mmc). После запуска приложения следует добавить остастку "Сертификаты". Чаще всего добавляются оснастки "Сертификаты - текущий пользователь" и "Сертификаты - текущий компьютер".

В форме просмотра сертификата есть три закладки: "Общие" (предназначение, срок действия и наличие закрытого ключа), "Состав" (различные атрибуты) и "Путь сертификации" (какие Root CA и промежуточный сертификаты используются).

Анализ атрибутов сертификата может помочь разобраться с причинами, по которым сертификат может быть расценен как не пригодный к использованию. Факторы, на которые следует обратить внимание в первую очередь:

- Время действия сертификата (не просрочен ли он)
- Кому выдан сертификат (имя host-а)
- Алгоритм подписи и hash-алгоритм подписи
- Издатель (например: CN=CA-INET;DC=msk;DC=shq)
- Субъект - для кого выдан ключ (например: E=isp@dors.com), или для какого сервера выдан ключ (например: CN=localhost)
- Параметры ключа
- Отпечаток ключа (_Thumbprint_)
- Friendly name (например: ASP.NET Core HTTPS development certificate)

## Параметры curl-запроса для передачи клиентского сертификата серверу

Экспериментально подтверждённый набор параметров для передачи клиентского сертификата серверу:

``` cpp
struct curl_blob clientCertBlob;

clientCertBlob.data = (void*)lpcszClientCertificate;
clientCertBlob.len = strlen(lpcszClientCertificate);

// Устанавливаем клиентский сертификат
curl_easy_setopt(curl, CURLOPT_SSLCERT_BLOB, &clientCertBlob);
// Устанавливаем клиентский сертификат из файла (для отладки)
//curl_easy_setopt(curl, CURLOPT_SSLCERT, "c:/Temp/badssl.com-client.pem");

// Указываем пароль для расшифровки private-ключа клиентского сертификата
curl_easy_setopt(curl, CURLOPT_KEYPASSWD, "badssl.com");
```

## Ошибки при настройке проверки клиентского сертификата в Windows

Не смотря на то, что клиентский сертификат может быть настроен корректно, при попытке подключения к серверу, IIS может возвращать код ответа 403.16. В этом ситуации, рекомендуется ознакомиться со статьёй: https://learn.microsoft.com/ru-ru/troubleshoot/developer/webapps/iis/health-diagnostic-performance/http-403-forbidden-access-website

В конкретной проблемной ситуаций была выполнена следующая команда PowerShell:

``` powershell
Get-Childitem cert:\LocalMachine\root -Recurse | Where-Object {$_.Issuer -ne $_.Subject} | Format-List * | Out-File "c:\computer_filtered.txt"
```

В файле "c:\computer_filtered.txt" появилась информация о том, что в списке доверенных корневых центров сертификации появился самоподписанный сертификат "Admin.pfx" (был сгенерирован корпоративной PKI). После того, как этот сертификат был удалён, проверка клиентского сертификата как средствами IIS, так и приложением ASP.NET Core 6, завершилась успехом.

Вот, что делает приведённая выше команда:

- `Get-Childitem cert:\LocalMachine\root -Recurse` - получает все дочерние элементы из хранилища сертификатов доверенных корневых центров сертификации локальной машины, применяя рекурсивный механизм
- `Where-Object {$_.Issuer -ne $_.Subject}` - условие отбора. Ключ -NE указывает на то, что Issuer (издатель) не соответствует Subject (субъекту). Поскольку рассматриваются только сертификаты корневых центров сертификации, у сертификатов добавленных Windows Issuer и Subject совпадают. Если Issuer и Subject не совпадают, то такой сертификат не может быть сертификатом в корневом центре сертификации, т.к. его подписал кто-то другой. Такой сертификат признаётся не доверенным и не может быть использован
- `Format-List` - указывает формат вывода данных. Звездочка указывает на то, что выводить нужно всё. Свойства объектов выводятся по одному в строке
- `Out-File "c:\computer_filtered.txt"` - указывает имя файла, в который следует сохранить сформированные данные
- символ `|` называется **pipeline** и означает, что результат предыдущей команды нужно передать следующей команде

### Покупка сертфикатов - особенности процесса

Некоторые организации предоставляют пробный срок в размере 5-14 дней. При этом сертификат предоставлятся со сроком действия в год/два/три, но если заказчик сертификат не оплачивает, то его отзывают.

Таким образом, следует иметь ввиду, что вы, как заказчик, сразу получаете полноценный, промышленный сертификат. Пример компании, которая предлагает SSL-сертификаты в России [LeaderSSL](https://www.leaderssl.ru/).

### Особенность покупки клиентского сертификата

Клиентский сертификат привязан к почтовому адресу. Соответственно, получить сертификат может только легальный владелец почтового адреса, либо системный администратор. Ввиду санкционных ограничений, получить сертификат для российского сегмента интернет нельзя. В случае ДОРС, получать сертификаты можно на @dors.com
