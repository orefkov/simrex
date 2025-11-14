# simrex - регулярные выражения для строк Simstr на базе Oniguruma
[![CMake on multiple platforms](https://github.com/orefkov/simrex/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/orefkov/simrex/actions/workflows/cmake-multi-platform.yml)

Предназначена для работы с регулярными выражениями при использовании библиотеки [simstr](https://github.com/orefkov/simstr).
В качестве движка регулярных выражений используется [Oniguruma](https://github.com/kkos/oniguruma).

Версия 1.0.0

В этой библиотеке содержится простая реализация простого объекта OnigRegexp для работы с регулярными выражениями
посредством движка `Oniguruma` с использование строковых объектов библиотеки *simstr*.

## Основные возможности библиотеки
- Работает со всеми строками simstr.
- Поддерживает работу со строками `char`, `char16_t`, `char32_t`, `wchar_t`.
- Различные виды поиска и замены.

## Основные объекты библиотеки
- OnigRegexp<K> - регулярное выражение, параметр К задаёт тип используемых символов в строке. Алиасы:
  - OnigRex - для строк char
  - OnigRexU - для строк char16_t
  - OnigRexUU - для строк char32_t
  - OnigRexW - для строк wchar_t

## Использование
`simrex` состоит из заголовочного файла и одного исходника. Можно подключать как CMake проект через `add_subdirectory` (библиотека `simrex`),
можно просто включить файлы в свой проект. Для сборки также требуется [simstr](https://github.com/orefkov/simstr) (при использовании CMake
скачивается автоматически).

Для работы `simrex` требуется компилятор с поддержкой стандарта не ниже С++20 (используются концепты).

## Описание возможностей Oniguruma
[Синтаксис выражений](https://github.com/kkos/oniguruma/blob/master/doc/RE)

## Примеры использования
```cpp
OnigRex rex{"b(a+)"};
auto matches = rex.all_matches("bbbaabbbabbaaa");
for (const auto& match: matches) {
    std::cout << "Found: ";
    unsigned group = 0;
    for (const auto [pos, text]: match) {
        if (group == 0) {
            std::cout << "at pos " << pos << ", text: " << text << std::endl;
        } else {
            std::cout << "  subgroup " << group  << ", pos " << pos
                << ", text: " << text << std::endl;
        }
        group++;
    }
}
```

Результат:
```
Found: at pos 2, text: baa
  subgroup 1, pos 3, text: aa
Found: at pos 7, text: ba
  subgroup 1, pos 8, text: a
Found: at pos 10, text: baaa
  subgroup 1, pos 11, text: aaa
```

## Сгенерированная документация
[Находится здесь](https://snegopat.ru/simrex/docs/)
