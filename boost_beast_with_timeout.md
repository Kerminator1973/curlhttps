# Асинхронное выполнение http-запросов с ограничением времени выполнения операции

Оригинальный пример взят с [официального репозитация boost](https://github.com/boostorg/beast/blob/develop/example/http/client/async/http_client_async.cpp)


Заголовочный файл "asyncHttpSession.h":

```cpp
#pragma once

#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


// Задача класса: выполнить асинхронный PUT-запрос с ограничением времени выполнения (time-out)
class asyncHttpSession : public std::enable_shared_from_this<asyncHttpSession>
{
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;         // (Критичное замечание: экземпляр должен быть сохранён между операциями чтения)
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    int error_code;                     // Код ошибки при выполнении операции

public:
    inline int getErrorCode() {
        return error_code;
    }

public:
    // Объект создаётся с использованием strand для того, чтобы обработчики событий
    // не выполнялись параллельно (concurrently)
    explicit asyncHttpSession(net::io_context& ioc)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc))
        , error_code(-7)
    {
    }

    // Метод позволяет запусть асинхронную операцию
    void startRequest( char const* host, char const* target, char const* body);

    void on_resolve( beast::error_code ec, tcp::resolver::results_type results );
    void on_connect( beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void on_write( beast::error_code ec, std::size_t bytes_transferred );
    void on_read( beast::error_code ec, std::size_t bytes_transferred );
};
```

Реализация:

```cpp

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "asyncHttpSession.h"

// Выводим в консоль сообщение об ошибке выполнения операции
void fail(beast::error_code ec, char const* what)
{
    std::cerr << "Ошибка на этапе: " << ec.message() << "\n";
}

// Запускаем асинхронную операцию
void asyncHttpSession::startRequest( char const* host, char const* target, char const* body)
{
    // Настраиваем HTTP-запрос с отправкой данных на сервер
    req_.version(11);               // HTTP 1.1
    req_.method(http::verb::put);   // PUT-verb
    req_.target(target);

    // Без указания поля "Host", тестовый узел на Node.js не будет отрабатывать запрос.
    // Подключение к Node.js имеет смысл для тестирования кода, который выполняет несколько
    // попыток отправки команды переключения режима аудита
    req_.set(http::field::host, host);
    req_.set(http::field::content_type, "application/json");

    // Формируем payload - документ, отправляемый прибору BVS
    req_.body() = body;

    // Вызов метода prepare_payload() позволяет добавить в заголовок
    // запроса такие поля, как "Content-Length"
    req_.prepare_payload();

    // Пытаемся определить адрес указанного хоста
    resolver_.async_resolve( host, "80", beast::bind_front_handler(
        &asyncHttpSession::on_resolve, shared_from_this()));
}

void asyncHttpSession::on_resolve( beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return fail(ec, "resolve");

    // Ограничиваем время получения IP-адреса хоста тремя секундами
    stream_.expires_after(std::chrono::seconds(3));

    // После получения IP-адреса осуществляем попытку подключения к D820
    stream_.async_connect( results,
        beast::bind_front_handler(
            &asyncHttpSession::on_connect,
            shared_from_this()));
}

void asyncHttpSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec)
        return fail(ec, "connect");

    // Ограничиваем время выполнения операции двумя секундами
    stream_.expires_after(std::chrono::seconds(2));

    // Посылаем HTTP-запрос на D820
    http::async_write(stream_, req_,
        beast::bind_front_handler(
            &asyncHttpSession::on_write,
            shared_from_this()));
}

void asyncHttpSession::on_write( beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Запускаем команду считывания результатов выполнения команды
    http::async_read(stream_, buffer_, res_,
        beast::bind_front_handler(
            &asyncHttpSession::on_read,
            shared_from_this()));
}

void asyncHttpSession::on_read( beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "read");

    // Проверяем код завершения HTTP-запроса
    auto code = res_.result_int();

    // Закрываем соединение, выполняя "Gracefully close the socket"
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    // Проверяем, что с завершением соединения не возникло каких-либо проблем
    if (ec && ec != beast::errc::not_connected)
        return fail(ec, "shutdown");

    // Если код ответа отличается от хорошего ответа, выводим ответ в консоль ошибок
    if (code != 200) {
        std::cerr << res_ << std::endl;
        return;
    }

    // Если мы пришли в эту строку кода, то это означает, что запрос был успешно выполнен,
    // а соединение закрыто

    // Устанавливаем код завершения, как успешный
    error_code = 0;
}
```

Пример использования:

```cpp
int PutRequestToD820Async(const std::string& strIPAddr, const std::string& reqBody, const std::string& resource)
{
    // I/O контекст, необходимый для всех I/O операций
    boost::asio::io_context ioc;

    // Запускаем асинхронную операцию
    auto session = std::make_shared<asyncHttpSession>(ioc);
    session->startRequest(strIPAddr.c_str(), resource.c_str(), reqBody.c_str());

    // Запускаем сервис ввода/вывода, которые завершится в момент, когда все асинхронные задачи будут выполнены
    ioc.run();

    return session->getErrorCode();
}
```
