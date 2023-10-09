#include <iostream> // cout
#include <clocale>  // setlocale
#include <boost/asio.hpp>
#include <boost/array.hpp>


using boost::asio::ip::tcp;

// Первый вариант клиентского приложения взят с сайта:
// https://www.codingwiththomas.com/blog/boost-asio-server-client-example
//
// Результат: мы попадаем в обработчик запроса и отваливаемся на то, что отсутствует
// корректный заголовок - 4 байта в сетевом порядке, в которых указывается длина данных

// Convert a wide Unicode string to an UTF8 string
// Исходник: https://gist.github.com/pezy/8571764
std::string utf8_encode(const std::wstring &wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

// Convert an ANSI string to a wide Unicode String
std::wstring ansi2unicode(const std::string &str)
{
  // Первый параметр можно заменить на 1251, что позволит конвертировать в кириллицу
  // на машинах, на которых установлена версия Windows, отличная от Русской
	int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

// Convert an wide Unicode string to ANSI string
std::string unicode2ansi(const std::wstring &wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

int main(int argc, char* argv[])
{
    std::setlocale(LC_ALL, "Russian");
    std::locale loc(".1251");	// Указываем кодовую страницу
    std::locale::global(loc);

    using boost::asio::ip::tcp;
    boost::asio::io_context io_context;
    
    // we need a socket and a resolver
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    
    // now we can use connect(..)
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", "22222"));
    
    // and use write(..) to send some data which is here just a string
    std::string message1251 {R"(<?xml version="1.0" encoding="utf-16"?>
<Request xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" type="Login">
  <Login>Администратор</Login>
  <Password>0E-48-05-72-6B-62-56-47-FA-3D-A2-01-00-BE-72-B5</Password>
</Request>)"};

    auto data = utf8_encode(ansi2unicode(message1251));
    
    // Формируем заголовок документа
    uint32_t input = htonl(data.length());
    auto result = boost::asio::write(socket, boost::asio::buffer(&input, 4));
    result = boost::asio::write(socket, boost::asio::buffer(data));
    
    // the result represents the size of the sent data
    std::cout << "data sent: " << data.length() << '/' << result << std::endl;

    // The boost::asio::buffer() function automatically determines 
    // the size of the array to help prevent buffer overruns.
    boost::array<char, 8192> buf;
    boost::system::error_code error;

    size_t len = socket.read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof) {
      std::cout << "error!" << std::endl;
    } else {

      std::cout << "received: " << len << " bytes" << std::endl;

      if (len >= 4) {

        uint32_t realInput = ntohl(*((u_long*)buf.data()));
        std::cout << "ntohl: " << realInput << " bytes" << std::endl;

        std::string content(buf.data() + 4, buf.data() + len);
        std::string ansi = unicode2ansi(utf8_decode(content));
        std::cout << ansi << std::endl;
      }
    }

    // and close the connection now
    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close();

    return 0;
}
