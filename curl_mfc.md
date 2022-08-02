# Потенциальные проблемы при сборке MFC-приложений с curl

При сборке MFC-приложения, использующего curl могут возникнуть следующие проблемы:

``` log
1>MSVCRT.lib(MSVCR120.dll) : error LNK2005: _strchr already defined in libcmt.lib(strchr.obj)
1>MSVCRT.lib(MSVCR120.dll) : error LNK2005: _strncpy already defined in libcmt.lib(strncpy.obj)
```

Причина проблемы - библиотеки были собраны в модели "без MFC", а основное приложение - в модели "с MFC".

Всего в Visual Studio используются четыре модели:

- **libcmt.lib**: static CRT link library for a release build (/MT)
- **libcmtd.lib**: static CRT link library for a debug build (/MTd)
- **msvcrt.lib**: import library for the release DLL version of the CRT (/MD)
- **msvcrtd.lib**: import library for the debug DLL version of the CRT (/MDd)

В случае, если собирается консольное приложение без MFC, компиляция завершается корректно. Единственный нюанс - при загрузке ресурса в корпоративной сети КБ ДОРС, необходимо явным образом указывать proxy-сервер, либо переходить на ресурс, размещённый в корпоративной сети.

Чтобы задавить различия в используемых runtime-ах, можно использовать совет, [найденный в интернет](https://stackoverflow.com/questions/15919435/include-curl-in-mfc-project):

``` text
By default, curl compiles with the /MD (dll) run-time library (that's why you get the __imp__ prefix to the unresolved externals).

I assume you build your MFC project with /MT (statically linked run-time).

You can build curl with static run-time by setting the RTLIBCFG environment variable in your build console to static, like this:

set RTLIBCFG=static

Or just add manually /MT to the compiler flags and /NODEFAULTLIB:MSVCRT.lib to the link flags in the makefiles, in case the first suggestion doesn't work.
```

Если задавить статическую CRT-библиотку ключём сборки `/NODEFAULTLIB:libcmt.lib`, то при сборке MFC-приложения может возникнуть ошибка:

``` output
error LNK2001: unresolved external symbol ___argc
error LNK2001: unresolved external symbol ___wargv
```

Ошибка устраняется установкой режима использования MFC "Use MFC in a Shared DLL".

Как результат, MFC-приложение может использовать библиотеку curl.
