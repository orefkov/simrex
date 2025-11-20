# simrex - Regular expressions for Simstr strings based on Oniguruma
[![CMake on multiple platforms](https://github.com/orefkov/simrex/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/orefkov/simrex/actions/workflows/cmake-multi-platform.yml)

Intended for working with regular expressions when using the [simstr](https://github.com/orefkov/simstr) library.
[Oniguruma](https://github.com/kkos/oniguruma) is used as the regular expression engine.

Version 1.0.0

This library contains a simple implementation of a simple OnigRegexp object for working with regular expressions
using the `Oniguruma` engine with the use of string objects from the *simstr* library.

## Key features of the library
- Works with all simstr strings.
- Supports working with `char`, `char16_t`, `char32_t`, `wchar_t` strings.
- Various types of search and replace.

## Main objects of the library
- OnigRegexp<K> - regular expression, parameter K specifies the type of characters used in the string. Aliases:
  - OnigRex - for char strings
  - OnigRexU - for char16_t strings
  - OnigRexUU - for char32_t strings
  - OnigRexW - for wchar_t strings

## Usage
`simrex` consists of a header file and one source file. You can connect as a CMake project via `add_subdirectory` (the `simrex` library),
you can simply include the files in your project. [simstr](https://github.com/orefkov/simstr) is also required for building (when using CMake,
it is downloaded automatically).

`simrex` requires a compiler with support for the C++20 standard or higher (concepts are used).

## Description of Oniguruma features
[Expression syntax](https://github.com/kkos/oniguruma/blob/master/doc/RE)

## Usage examples
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

Result:
```
Found: at pos 2, text: baa
  subgroup 1, pos 3, text: aa
Found: at pos 7, text: ba
  subgroup 1, pos 8, text: a
Found: at pos 10, text: baaa
  subgroup 1, pos 11, text: aaa
```

## Generated documentation
[Located here](https://snegopat.ru/simrex/docs/)
