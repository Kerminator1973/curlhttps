# Пример кода для взаимодействия с сервером по TCP в асинхронном режиме

Асинхронный режим гораздо более эффективно использует вычислительный ресурс процессора.

Первый вариант клиентского приложения взят с [сайта](https://www.codingwiththomas.com/blog/boost-asio-server-client-example). Одно из замечаний к заимстрованному приложению: класс [io_service является устаревшим](https://stackoverflow.com/questions/59753391/boost-asio-io-service-vs-io-context). Вместо него нужно использовать **io_context**.

Для реализации процесса взаимодействия рекомендуется разработать специализированный класс, например, **TCPClient**. Заголовочный файл "tcpClientCmd.h" может выглядеть следующим образом:

```cpp
#pragma once
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

class TCPClient
{
    public:
        TCPClient(boost::asio::io_context& ioContext, tcp::resolver::iterator EndPointIter);

        // Метод добавляет в очередь задач асинхронную задачу завершения операции
        void Close();

    private:
        // Ссылка на очередь асинхронных операций. Используется для добавления
        // в очередь новых задач
        boost::asio::io_context& m_ioContext;

        tcp::socket m_Socket;

        // Буфер, содержит строку, передаваемую на сервер ЭСКД ProIDC
        std::string m_SendBuffer;

        // Устанавливаем буфер размером 8K - этого должно хватить для получения списка
        // пользовательских ролей.
        // TODO: реализовать докачку сообщений (топологическая схема может быть очень большой)
        boost::array<char, 8192> m_RecieveBuffer;

        // Callback-методы, отрабатывающие задачи операции выполнения http-запроса
        void OnConnect(const boost::system::error_code& ErrorCode, tcp::resolver::iterator EndPointIter);
        void OnReceive(const boost::system::error_code& ErrorCode, std::size_t readBytes);
        void OnSend(const boost::system::error_code& ErrorCode);
        void DoClose();
};```

Реализация класса, в файле "tcpClientCmd.cpp" может выглядеть так:

```cpp
#include <iostream>             // cout
#include <boost/bind/bind.hpp>  // boost::bind()
#include "helpers.h"
#include "converters.h"
#include "tcpClientCmd.h"

using boost::asio::ip::tcp;


TCPClient::TCPClient(boost::asio::io_context& ioContext, tcp::resolver::iterator EndPointIter)
: m_ioContext(ioContext), m_Socket(ioContext), m_SendBuffer(""), m_RecieveBuffer()
{
    tcp::endpoint EndPoint = *EndPointIter;
    m_Socket.async_connect(EndPoint,
        boost::bind(&TCPClient::OnConnect, this, boost::asio::placeholders::error, ++EndPointIter));
}

void TCPClient::Close()
{
    m_ioContext.post( boost::bind(&TCPClient::DoClose, this));
}

void TCPClient::OnConnect(const boost::system::error_code& ErrorCode, tcp::resolver::iterator EndPointIter)
{
    std::cout << "OnConnect..." << std::endl;
    if (!ErrorCode)
    {
        std::string strUsername = "Администратор";
        std::string strPassword = "38Gjgeuftd_";

        auto strLoginXml = GetLoginXml(strUsername, strPassword);

        // Заголовок документа - 32 битное число в сетевом порядке, в котором передаётся длина пакета
        uint32_t input = htonl(strLoginXml.length());

        m_SendBuffer = std::string((char*)&input, 4);
        m_SendBuffer += strLoginXml;

        m_Socket.async_send(boost::asio::buffer(m_SendBuffer.c_str(),m_SendBuffer.length()),
            boost::bind(&TCPClient::OnSend, this,
            boost::asio::placeholders::error));
    } 
    else if (EndPointIter != tcp::resolver::iterator())
    {
        // Если не удалось установить соединение с Endpoint, пробуем использовать следующий Endpoint.
        m_Socket.close();
        tcp::endpoint EndPoint = *EndPointIter;

        m_Socket.async_connect(EndPoint, 
            boost::bind(&TCPClient::OnConnect, this, boost::asio::placeholders::error, ++EndPointIter));
    }
}

void TCPClient::OnSend(const boost::system::error_code& ErrorCode)
{
    if (!ErrorCode)
    {
        std::cout << "\"" << m_SendBuffer << "\" has been sent" << std::endl;

        m_Socket.async_read_some(boost::asio::buffer(m_RecieveBuffer),
            boost::bind(&TCPClient::OnReceive, this, 
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
            );
    }
    else
    {
        std::cout << "OnSend closing" << std::endl;
        DoClose();
    }
}

void TCPClient::OnReceive(const boost::system::error_code& ErrorCode, std::size_t readBytes)
{
    if (!ErrorCode)
    {
        std::cout << "Read bytes:" << readBytes << std::endl;

        if (readBytes >= 4) {

            uint32_t realInput = ntohl(*((u_long*)m_RecieveBuffer.data()));
            std::cout << "ntohl: " << realInput << " bytes" << std::endl;

            std::string content(m_RecieveBuffer.data() + 4, m_RecieveBuffer.data() + readBytes);
            std::string ansi = unicode2ansi(utf8_decode(content));
            std::cout << ansi << std::endl;
        }
    } 
    else 
    {
        std::cout << "ERROR! OnReceive..." << ErrorCode.message() << std::endl;
        DoClose();
    }
}

void TCPClient::DoClose()
{
    boost::system::error_code ec;
    m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_Socket.close();
}
```

В приведённом здесь примере отсутствуют заголовочные файлы "helpers.h" и "converters.h", а также файлы реализации, но они являются специфическими для конкретной реализации (подключения к серверу ЭСКД ProIDC).

Запустить задачу на исполнение можно так:

```cpp
#include <iostream>             // cout
#include <clocale>              // setlocale
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>  // boost::bind()
#include <boost/thread.hpp>
#include "tcpClientCmd.h"

#include <boost/uuid/detail/md5.hpp>

using boost::asio::ip::tcp;


int main(int argc, char* argv[])
{
    std::setlocale(LC_ALL, "Russian");
    std::locale loc(".1251");
    std::locale::global(loc);

    try 
    {
        std::cout << "Client is starting..." << std::endl;

        // Создаём очередь для выполнения асинхронных операций
        boost::asio::io_context io_Context;

        // Выполняем запрос на поиск Endpoints (IP-адресов) по указанному URL и порту.
        // Гипотетически, результирующих Endpoints может быть несколько и нам следует
        // использовать тот, который является активным (т.е. сможет ответить на выполнение
        // операции connect)
        tcp::resolver Resolver(io_Context);
        tcp::resolver::query Query("127.0.0.1", "22222");
        tcp::resolver::iterator EndPointIterator = Resolver.resolve(Query);

        // Создаём объект, который реализует процесс взаимодействия с сервером целиком
        TCPClient Client(io_Context, EndPointIterator);

        std::cout << "Client is started!" << std::endl;

        // Связываем поток исполнения и запускаем в нём очередь выполнения асинхронных запросов
        boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &io_Context));

        // Ждём завершения потока и закрываем сетевое соединение
        ClientThread.join();
        Client.Close();
    } 
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    std::cout << "\nClosing";

    return 0;
}
```

## Зачем нужно запускать поток исполнения и связывать его с Executor-ом

[Executor](https://alandefreitas.github.io/moderncpp/programming-paradigms/parallelism/executors/)-ом называется экземпляр класса boost::asio::**io_context**, boost::asio::**io_service**. В приведённом выше коде, имя соответствующего исполнителя - m_IOService.

Если мы запускаем исполнителя в отдельном потоке, мы можем использовать поток пользовательского интерфейса для взаимодействия с пользователем, например:

```cpp
// Связываем поток исполнения и запускаем в нём очередь выполнения асинхронных запросов
boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &io_Context));

// Ждём ввода команды. Команда x - позволяет остановить работу приложения
for (;;) {
    std::string s;
    std::cin >> s;
    if (s.compare("x") == 0) {
        std::cout << "Quitting...\"\n";

        Client.Cancel();
        break;
    }
}

// Ждём завершения потока и закрываем сетевое соединение
ClientThread.join();
Client.Close();
```

Отмена последней асинхронной операции может быть реализована следующим образом:

```cpp
void TCPClient::Cancel()
{
    m_ioContext.post(boost::bind(&TCPClient::DoCancel, this));
}

void TCPClient::DoCancel()
{
    m_Socket.cancel();
}
```

Подобное поведение может иметь смысл, когда соединение между сервером и клиентом не разрывается, обеспечивая асинхронный обмен сообщениями:

```cpp
void TCPClient::OnReceive(const boost::system::error_code& ErrorCode, std::size_t readBytes)
{
    if (!ErrorCode)
    {
        std::cout << "Read bytes:" << readBytes << std::endl;

        if (readBytes >= 4) {

            uint32_t realInput = ntohl(*((u_long*)m_RecieveBuffer.data()));
            std::cout << "ntohl: " << realInput << " bytes" << std::endl;

            std::string content(m_RecieveBuffer.data() + 4, m_RecieveBuffer.data() + readBytes);
            std::string ansi = unicode2ansi(utf8_decode(content));
            std::cout << ansi << std::endl;
        }

        // Для механизма подписки на события, посылаем команду получения данных от
        // контроллера ещё раз
        m_Socket.async_read_some(boost::asio::buffer(m_RecieveBuffer),
            boost::bind(&TCPClient::OnReceive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    } 
    else 
    {
        std::cout << "ERROR! OnReceive..." << ErrorCode.message() << std::endl;
        DoClose();
    }
}
```
