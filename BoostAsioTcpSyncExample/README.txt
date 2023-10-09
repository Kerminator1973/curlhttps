# Сборка проекта

Команды сборки приложения:

```shell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Примеры использования Boost.asio

Примеры кода на Boost.asio: https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/examples.html

Асинхронный клиент: https://itecnote.com/tecnote/c-boost-async-tcp-client/

Синхронный клиент:
https://www.bogotobogo.com/cplusplus/Boost/boost_AsynchIO_asio_tcpip_socket_server_client_timer_bind_handler_multithreading_synchronizing_network_D.php
https://www.bogotobogo.com/cplusplus/Boost/boost_AsynchIO_asio_tcpip_socket_server_client_timer_A.php

Reconnecting socket (стрёмный, но тема хорошая): https://codereview.stackexchange.com/questions/270333/a-reuseable-reconnecting-tcp-socket-with-asio

Можно ещё и вот это почитать: https://stackoverflow.com/questions/76014340/boost-asio-synchronization-between-tcp-client-and-server

Исходники из книги по Boost.Asio, которую я прочитал: https://github.com/wyrover/book-code/tree/master/Boost.Asio%20C%2B%2B%20Network%20Programming%2C%202nd%20Edition

Пример находится в архиве «4601_06_Codes.zip\SourceCode_B04601_06», приложение называется echoserver и описывается в шестой главе.

Там же есть исходники из ещё нескольких книг по Boost.Asio.
