# Сжатие данные в приложении на C++ с использованием Boost и zlib

Подключаемые заголовочные файлы:

```  cpp
#include <iostream>
#include <boost/iostreams/device/back_inserter.hpp> // Нужно для архивации данных
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp> // Нужно для распаковки архива
```

## Сжать данные из потока

В приведённом ниже примере источником данных является ansi-строка lpcszRootCertificate:

``` cpp
using namespace boost::iostreams;

std::vector<char> v;
back_insert_device<std::vector<char>> snk{ v };

// Создаём поток в котором будет применяться фильтр для сжатия данных ZLIB.
filtering_ostream os;
os.push(zlib_compressor{});
os.push(snk);

os << lpcszRootCertificate;
os << std::flush;
os.pop();
```

В результате выполнения кода вектор v заполняется данными, представляющими собой полноценной архив.

## Распаковать архив в поток

Для распаковки данных, представленных массивом в выходной поток можно следующим программным кодом:

``` cpp
using namespace boost::iostreams;

array_source src { (char*)lpcstrZippedRootCA, sizeof(lpcstrZippedRootCA) };
filtering_istream filteringIS;
filteringIS.push(zlib_decompressor{});
filteringIS.push(src);

// Распаковываем данные и помещаем их в строку
std::ostringstream ss;
ss << filteringIS.rdbuf();
```

Этот код использует массив lpcstrZippedRootCA с фиксированной длиной. Для того, чтобы получить строки из потока filteringIS достаточно вызывать метод str(): `ss.str()`
