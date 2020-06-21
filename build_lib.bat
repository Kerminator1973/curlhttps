REM КЛЮЧЕВЫМ РЕЗУЛЬТАТОМ СБОРКИ ЯВЛЯЕТСЯ БИБЛИОТЕКА:
REM c:\curl\builds\libcurl-vc12-x86-debug-static-zlib-static-obj-lib\libcurl_a_debug.lib 

echo off

REM Предусловие - инструментарий git должен быть установлен: https://git-scm.com/downloads
echo Downloading the source files of the Curllib from the GitHub

pushd .

REM Мы созадём папку в корневом подкаталоге
cd "c:\"

git clone https://github.com/curl/curl.git
REM Результат - должны быть загружены исходные тексты библиотеки в папку "curl"
REM После сборки исходников, папку можно будет удалить для экономии места на диске

cd curl

REM Запускаем конфигурационный скрипт
echo Running the buildconf.bat
call c:\curl\buildconf.bat

REM Копируем собранные в ПроАТМ артефакты в отдельную папку на диске C:/
echo Copying the existing files of the Zlib from the ProATM's subfolders
if not exist "depends_debug" mkdir depends_debug
cd depends_debug
if not exist "include" mkdir include
if not exist "lib" mkdir lib
if not exist "bin" mkdir bin
popd

copy /Y ..\Common\zlib\*.h c:\curl\depends_debug\include\*
copy /Y Debug\zlib.lib c:\curl\depends_debug\lib\*

pushd .

cd c:\curl\winbuild

rem echo Setting up the environment (Microsoft Visual Studio 2013 Tools)
call "%VS120COMNTOOLS%\vsvars32.bat"

REM Пока не активирован SSL: WITH_SSL=static SSL_PATH=..\depends_debug
rem echo Building the CUrl (Debug mode)
call nmake /f Makefile.vc mode=static DEBUG=yes VC=12 WITH_ZLIB=static ZLIB_PATH="..\depends_debug" ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no ENABLE_WINSSL=no

popd

echo Successfully Finished