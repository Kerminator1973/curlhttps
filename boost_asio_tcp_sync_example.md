# Пример кода для взаимодействия с сервером по TCP с синхронном режиме

Рабочий пример взаимодействия с ProIDC Server приведён в примере в папке "BoostAsioTcpSyncExample". Пример демонстрирует использование Boost.asio в синхронном режиме, что является хорошей отправной точкой, но не промышленным подходом.

Заголовочные файлы:

```cpp
#include <iostream> // cout
#include <clocale>  // setlocale
#include <boost/asio.hpp>
#include <boost/array.hpp>
```

Указываем endpoint (127.0.0.1:22222) сервера, к которому планируем подключиться:

```cpp
using boost::asio::ip::tcp;
boost::asio::io_context io_context;

// we need a socket and a resolver
tcp::socket socket(io_context);
tcp::resolver resolver(io_context);

// now we can use connect(..)
boost::asio::connect(socket, resolver.resolve("127.0.0.1", "22222"));
```

Посылает сообщение на сервер. В рамках протокола взаимодействия с ProIDC Server, сообщение начинается с четырёх-байтового значения длины (в **network byte order**), данные представляют собой XML в формате UFT8:

```cpp
// and use write(..) to send some data which is here just a string
std::string message1251 {R"(<?xml version="1.0" encoding="utf-16"?>
<Request xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" type="Login">
<Login>Администратор</Login>
<Password>0E-48-05-72-6B-62-56-47-FA-3D-A2-01-00-BE-72-B5</Password>
</Request>)"};

auto data = utf8_encode(ansi2unicode(message1251));

// Формируем заголовок документа и пересылаем его на сервер
uint32_t input = htonl(data.length());
auto result = boost::asio::write(socket, boost::asio::buffer(&input, 4));
// Отправляем на сервер XML-документ (полезная нагрузка)
result = boost::asio::write(socket, boost::asio::buffer(data));
```

Получает ответ сервера:

```cpp
boost::array<char, 8192> buf;
boost::system::error_code error;

size_t len = socket.read_some(boost::asio::buffer(buf), error);
if (error == boost::asio::error::eof) {
    std::cout << "error!" << std::endl;
} else {
```

Если получено больше 4 байт, т.е. если есть заголовок, то извлекаем полученный XML-документ:

```cpp
if (len >= 4) {
    uint32_t realInput = ntohl(*((u_long*)buf.data()));
    std::cout << "ntohl: " << realInput << " bytes" << std::endl;

    std::string content(buf.data() + 4, buf.data() + len);
    std::string ansi = unicode2ansi(utf8_decode(content));
    std::cout << ansi << std::endl;
}
```

Закрываем соединение:

```cpp
boost::system::error_code ec;
socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
socket.close();
```

## Вычисление хэша MD5

Следующий _snippet_ вычисляет хэша MD5 для логина и пароля в системе ЭСКД ProIDC:

```cpp
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using boost::asio::ip::tcp;
using boost::uuids::detail::md5;

std::string toString(const md5::digest_type &digest)
{
    const auto intDigest = reinterpret_cast<const int*>(&digest);
    std::string result;
    boost::algorithm::hex(intDigest, intDigest + (sizeof(md5::digest_type)/sizeof(int)), std::back_inserter(result));
    return result;
}
```

```cpp
// Вычисляем MD5-хэш из логина и пароля пользователя
md5 hash;
md5::digest_type digest;

// byte[] bytesPwdHash = md5.ComputeHash(UTF8Encoding.UTF8.GetBytes(UID + "_" + strRawPWD));
std::string s;
s = utf8_encode(ansi2unicode("Администратор_38Gjgeuftd_"));

hash.process_bytes(s.data(), s.size());
hash.get_digest(digest);

std::cout << "md5(" << s << ") = " << toString(digest) << '\n';
```

Следует заметить, что начиная с 1.70.0, [Boost не содержит реализации MD5](https://github.com/boostorg/uuid/issues/111), которая валидируется сетевыми инструментальными средствами. Заметим, что в коде используется библиотека boost::uuids, от которой не требуется совместимости с MD5, но которая использовала MD5 в качестве одного из компонентов своей реализации.

Однако, в моём конкретном примере, который собран с использованием 1.74, хэш-код совпал с генерируемым Microsoft.NET Framework 4.8.
