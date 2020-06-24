# Сборка библиотек Curl для встраивания в C++ приложения

Использовать библиотеку Curl для выполнения https-запросов с использованием самоподписанных сетификатов.

Сборка Curl должна осуществляться без вспомогательных утилит командной строки.

Целевая платформа: Microsoft Windows 7/10. Компиляторы: Visual Studio 2013 и 2019.

Решение рассматривается как альтернатива **Boost.Beast**.

## Начальная точка

Библиотека curllib зависит от других библиотек, в частности от zlib и openSSL.

Инструкция по сборке **curllib.lib** подробно описана в файле "\winbuild\BUILD.WINDOWS.txt" исходных текстов, доступных в [официальном репозитарии](https://github.com/curl/curl). Загрузить исходники можно и с [официального сайта](https://curl.haxx.se/download.html).

Актуальные [исходные тексты zlib](http://www.zlib.net/) и [openSSL](https://www.openssl.org/source/) могут быть загружены скриптом "**build_lib.bat**".

Исходные тексты библиотеки zlib также могут быть скачены из репозитария [GitHub от Madler - Mark Adler - соавтор zlib](https://github.com/madler/zlib).

## Подготовка к сборке

Чтобы собрать curllib можно использовать Visual Studio Tools. Следует открыть консоль операционной системы и выполнить инициализацию системы сборки. Выбрать подходящую строку инициализации можно нажав кнопку "Старт" -> "Visual Studio 2013" -> "Visual Studio Tools" и выбрав соответствующий скрипт. Для x86 (32-бита) используется следующая строка инициализации:

```
%comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"" x86
```

Сборка curllib.lib осуществляется из папки "\winbuild\" следующей командой:

```
nmake /f Makefile.vc mode=static DEBUG=yes 
  WITH_ZLIB=static ZLIB_PATH=c:\Projects\curl-7.70.0\deps 
  WITH_SSL=static SSL_PATH=c:\Projects\curl-7.70.0\deps
  ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no ENABLE_WINSSL=no  
```

Параметр "mode" определяет тип линковки (dll, статическая линковка). Флаг "DEBUG" позволяет указать, какую версию следует собирать (Debug/Release). Параметры WITH_ZLIB и ZLIB_PATH указывают на необходимость линковки **Zlib** и путь к исходникам. А параметры WITH_SSL и SSL_PATH - необходимость линковки **openSSL** и путь к исходникам. На начальном этапе можно собрать библиотеку без openSSL. Дополнительные параметры ENABLE_SSPI, ENABLE_IPV6 и ENABLE_WINSSL, по умолчанию, установлены в **yes** - этот фактор следует учитывать при сборке приложения.
