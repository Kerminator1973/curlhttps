# Разработка скрипта сборки в Windows

## Подготовительная часть

Задача - проверить параметры командной строки и вывести пояснения:

```
echo off

REM Предусловие - инструментарий git должен быть установлен: https://git-scm.com/downloads
REM
REM Первый параметр (%1) - целевой подкаталог, в который следует загрузить 
REM репозитарии исходных текстов: Curl, zlib и openSSL

REM Проверяем наличие параметров
IF "%~1" == "" GOTO EMPTY_ARGS

REM Проверяем, существует ли указанный подкаталог
IF EXIST %1 GOTO CLONE

REM Создаём указанный подкаталог (рекурсивно)
mkdir "%1"
```

## Загрузка исходных текстов

Задача - загрузить актуальные исходные тексты из репозитариев в интернет:

```
:CLONE
pushd .
cd "%1"
echo Downloading the source files of the GitHub
git clone https://github.com/curl/curl.git
git clone https://github.com/madler/zlib
git clone https://github.com/openssl/openssl 
popd
```

Команда **pushd** сохраняет на стеке текущий подкаталог, а **popd** - возвращается в ранее сохранённый подкаталог.

## Сборка исходных текстов

Скрипт для данного этапа пока не разработан, есть только вспомогательная часть:

```
REM Следующий этап - сборка исходных библиотеки из исходных текстов
:BUILD
echo TODO: compile everything

echo Finished successfully

GOTO EXIT

:EMPTY_ARGS

echo Please, specify the directory to store all required repositories.
echo Example:
echo 	build_lib.bat d:/Sources/

GOTO EXIT
```

Копирование результатов сборки - это тоже пока прототип кода:

```
REM КЛЮЧЕВЫМ РЕЗУЛЬТАТОМ СБОРКИ ЯВЛЯЕТСЯ БИБЛИОТЕКА:
REM c:\curl\builds\libcurl-vc12-x86-debug-static-zlib-static-obj-lib\libcurl_a_debug.lib 

REM cd curl

REM Запускаем конфигурационный скрипт
REM echo Running the buildconf.bat
REM call c:\curl\buildconf.bat

REM Копируем собранные в ПроАТМ артефакты в отдельную папку на диске C:/
REM echo Copying the existing files of the Zlib from the ProATM's subfolders
REM if not exist "depends_debug" mkdir depends_debug
REM cd depends_debug
REM if not exist "include" mkdir include
REM if not exist "lib" mkdir lib
REM if not exist "bin" mkdir bin
REM popd

REM copy /Y ..\Common\zlib\*.h c:\curl\depends_debug\include\*
REM copy /Y Debug\zlib.lib c:\curl\depends_debug\lib\*

REM pushd .

REM cd c:\curl\winbuild

rem echo Setting up the environment (Microsoft Visual Studio 2013 Tools)
REM call "%VS120COMNTOOLS%\vsvars32.bat"

REM Пока не активирован SSL: WITH_SSL=static SSL_PATH=..\depends_debug
rem echo Building the CUrl (Debug mode)
REM call nmake /f Makefile.vc mode=static DEBUG=yes VC=12 WITH_ZLIB=static ZLIB_PATH="..\depends_debug" ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no ENABLE_WINSSL=no

REM popd

REM echo Successfully Finished
```

## Завершающая часть скрипта 

```
:EXIT
```
