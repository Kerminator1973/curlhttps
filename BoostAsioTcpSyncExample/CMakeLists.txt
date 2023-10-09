# Минимальной версий CMake является 3.22. В действительности, никакая специфика
# версий CMake во внимание, пока, не принимается и могли бы быть использованы более
# ранние версии. В Ubuntu 22.04 устанавливается CMake 3.22, а на Windows-машинах
# КБ ДОРС используются CMake 3.24
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

# Имя проекта SberSync, или sbersync одной строкой
project(sbersync LANGUAGES CXX)

if (WIN32)
	# Исторически сложившаяся практика в ОП КБ ДОРС состоит в том, что Boost размещается
	# не стандартным образом, в не стандартных местах расположения на жёсстком диске и
	# функция find_package найти его не сможет. По этой причине, CMake нуждается в подсказке
	set( BOOST_ROOT c:/boost/ )
	set( BOOST_INCLUDEDIR c:/boost/include/boost-1_76/ )
	set( BOOST_LIBRARYDIR c:/boost/lib )

	# Указываем, что следует использовать статическую линковку RUNTIME 
	option(MSVC_CRT_STATIC "Link .exe with /MT instead of the default /MD" OFF)
endif (WIN32)

# Use static libraries of Boost
set( Boost_USE_STATIC_LIBS ON )

# В случае необходимости использования std::span<>, необходимо переключится на использование стандарта C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)	# Не использовать специализированных возможностей конкретного компилятора
set(CMAKE_CXX_STANDARD_REQUIRED ON)	# Компилятор должен соответствовать стандарту C++

# В Ubuntu 22.04 доступен Boost 1.74, а на Windows-машинах в КБ ДОРС используется Boost: 1.76
find_package(Boost 1.74 REQUIRED COMPONENTS system program_options locale filesystem)

# Указываем список файлов, участвующих в сборке проекта
add_executable(${PROJECT_NAME} main.cpp)

# Указываем подкаталог Boost-а, в котором следует искать его include-файлы
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})

if (WIN32)
	# Указываем нужные нам библиотеки Boost. Для работы boost.asio достаточно system
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif (WIN32)
