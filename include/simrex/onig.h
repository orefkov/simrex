/*
* (c) Проект "SimRex", Александр Орефков orefkov@gmail.com
* Регэкспы на oniguruma для SimStr.
*/
#pragma once
#include <simstr/sstring.h>
#include <oniguruma.h>
#include <list>
#include <optional>

#ifdef SIMREX_IN_SHARED
    #if defined(_MSC_VER) || (defined(__clang__) && __has_declspec_attribute(dllexport))
        #ifdef SIMREX_EXPORT
            #define SIMREX_API __declspec(dllexport)
        #else
            #define SIMREX_API __declspec(dllimport)
        #endif
    #elif (defined(__GNUC__) || defined(__GNUG__)) && defined(SIMSTR_EXPORT)
        #define SIMREX_API __attribute__((visibility("default")))
    #else
        #define SIMREX_API
    #endif
#else
    #define SIMREX_API
#endif

namespace simrex {
using namespace simstr;
using namespace simstr::literals;

struct OnigRegionDeleter {
    SIMREX_API void operator()(OnigRegion* region) const;
};
struct OnigRexDeleter {
    SIMREX_API void operator()(OnigRegex rex) const;
};

using RegexPtr = std::unique_ptr<OnigRegexType, OnigRexDeleter>;
using RegionPtr = std::unique_ptr<OnigRegion, OnigRegionDeleter>;

class OnigRegExpBase {
public:
    OnigRegExpBase(const OnigRegExpBase&) = delete;
    OnigRegExpBase& operator=(const OnigRegExpBase&) = delete;

    operator OnigRegex() const {
        return regexp_.get();
    }

    bool isValid() const {
        return (bool)regexp_;
    }

protected:
    OnigRegExpBase() = default;
    OnigRegExpBase(const OnigUChar* pattern, size_t length, OnigEncoding enc) : regexp_{create_regex(pattern, length, enc)} {}

    SIMREX_API static OnigRegex create_regex(const OnigUChar* pattern, size_t length, OnigEncoding enc);

    OnigRegExpBase(OnigRegExpBase&& other) noexcept = default;
    ~OnigRegExpBase() = default;
    OnigRegExpBase& operator=(OnigRegExpBase&& other) noexcept = default;

    SIMREX_API int search(const OnigUChar* start, size_t length, size_t offset) const;

    RegexPtr regexp_;
};

template<typename K>
struct RexTraits {
    static const OnigUChar* toChar(const K* ptr) {
        return reinterpret_cast<const OnigUChar*>(ptr);
    }
    static int toLen(size_t len) {
        return int(len * sizeof(K));
    }
    static const K* fromChar(const OnigUChar* ptr) {
        return reinterpret_cast<const K*>(ptr);
    }
    static size_t fromLen(int len) {
        return size_t(len / sizeof(K));
    }
};

/*!
 * @brief Класс для работы с oniguruma регэкспами
 * @tparam K - тип символов
 */
template<typename K>
class OnigRegexp : public OnigRegExpBase {
    using rt = RexTraits<K>;

public:
    using str_type = simple_str<K>;

    OnigRegexp() = default;
    OnigRegexp(OnigRegexp&& other) noexcept = default;
    ~OnigRegexp() = default;

    OnigRegexp(const OnigRegexp&) = delete;
    OnigRegexp& operator=(const OnigRegexp&) = delete;

    /*!
     * @brief Создает объект Onig Regexp.
     * @param pattern - регулярное выражение.
     */
    OnigRegexp(str_type pattern) : OnigRegExpBase(rt::toChar(pattern.symbols()), rt::toLen(pattern.length()), rex_encoding()) {}

    OnigRegexp& operator=(OnigRegexp&& other) noexcept = default;

    /*!
     * @brief Поиск положения первого вхождения.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return size_t - позицию найденного вхождения, -1, если не найдено.
     */
    size_t search(str_type text, size_t offset = 0) {
        int res = OnigRegExpBase::search(rt::toChar(text.symbols()), rt::toLen(text.length()), rt::toLen(offset));
        return res < 0 ? (size_t)res : rt::fromLen(res);
    }
    /*!
     * @brief Посчитать количество вхождений.
     * @param text - текст, в котором ищем.
     * @param maxCount - максимальное количество для ограничения поиска.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return количество найденных вхождений.
     */
    SIMREX_API size_t count_of(const str_type& text, size_t maxCount = -1, size_t offset = 0);
    /*!
     * @brief Текст первого найденного вхождения.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return T - текст найденного вхождения, или пустую строку, если не найдено.
     */
    template<StrType<K> T = str_type>
    T first_founded(str_type text, size_t offset = 0) const {
        return first_founded_str(text, offset);
    }
    /*!
     * @brief Получить тексты всех найденных вхождений, без разделения на подгруппы.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<simple_str<K>> - вектор с текстами всех найденных вхождений.
     */
    SIMREX_API std::vector<str_type> all_founded(str_type text, size_t offset = 0, size_t maxCount = -1) const;
    /*!
     * @brief Получить тексты всех найденных вхождений, без разделения на подгруппы, в заданном типе.
     * @tparam T - тип текста в возвращаемом результате.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<T> - вектор с текстами всех найденных вхождений.
     */
    template<StrType<K> T>
    std::vector<T> all_founded(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<T> matches;
        all_founded_str(text, offset, maxCount, &matches, [](str_type word, void* res) {
            static_cast<std::vector<T>*>(res)->emplace_back(word);
        });
        return matches;
    }
    /*!
     * @brief Получить текст первого найденного вхождения вместе с текстами подгрупп.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<simple_str<K>> - массив с текстами найденного вхождения - в первом элементе массива возвращает
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    SIMREX_API std::vector<str_type> texts_in_first_match(str_type text, size_t offset = 0) const;
    /*!
     * @brief Получить текст первого найденного вхождения вместе с текстами подгрупп.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<T> - массив с текстами найденного вхождения - в первом элементе массива возвращает
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    template<StrType<K> T>
    std::vector<T> texts_in_first_match(str_type text, size_t offset = 0) const {
        std::vector<T> matches;
        for_first_match(text, offset, &matches, [](OnigRegion* region, const OnigUChar *start, void* res) {
            std::vector<T>& matches = *static_cast<std::vector<T>*>(res);
            matches.reserve(region->num_regs);
            for (int i = 0; i < region->num_regs; i++) {
                matches.emplace_back(str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
            }
        });
        return matches;
    }
    /*!
     * @brief Получить тексты всех найденных вхождений вместе с подгруппами.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<std::vector<simple_str<K>>> - массив с массивами, в которых в первом элементе находится
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    SIMREX_API std::vector<std::vector<str_type>> texts_in_all_matches(str_type text, size_t offset = 0, size_t maxCount = -1) const;
    /*!
     * @brief Получить тексты всех найденных вхождений вместе с подгруппами.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<std::vector<T>> - массив с массивами, в которых в первом элементе находится
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    template<StrType<K> T>
    std::vector<std::vector<T>> texts_in_all_matches(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<std::vector<T>> matches;
        for_all_match(text, offset, maxCount, &matches, [](OnigRegion* region, const OnigUChar* start, void* res) {
            std::vector<std::vector<T>>& matches = *static_cast<std::vector<std::vector<T>>*>(res);
            auto& match = matches.emplace_back();
            match.reserve(region->num_regs);
            for (int i = 0; i < region->num_regs; i++) {
                match.emplace_back(str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
            }
        });
        return matches;
    }
    /*!
     * @brief Получить всю информацию о первом найденном вхождении.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<std::pair<size_t, simple_str<K>>> - массив, в котором первый элемент описывает всё вхождение, а следующие -
     *      подгруппы вхождения. Описание представляет собой пару, первый элемент которой - позиция начала вхождения,
     *      второй - текст вхождения.
     */
    SIMREX_API std::vector<std::pair<size_t, str_type>> first_match(str_type text, size_t offset = 0) const;
    /*!
     * @brief Получить всю информацию о первом найденном вхождении.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<std::pair<size_t, T>> - массив, в котором первый элемент описывает всё вхождение, а следующие -
     *      подгруппы вхождения. Описание представляет собой пару, первый элемент которой - позиция начала вхождения,
     *      второй - текст вхождения.
     */
    template<StrType<K> T>
    std::vector<std::pair<size_t, T>> first_match(str_type text, size_t offset = 0) const {
        std::vector<std::pair<size_t, T>> match;
        for_first_match(text, offset, &match, [](OnigRegion* region, const OnigUChar *start, void* res) {
            std::vector<std::pair<size_t, T>>& match = *static_cast<std::vector<std::pair<size_t, T>>*>(res);
            match.reserve(region->num_regs);
            for (int i = 0; i < region->num_regs; i++) {
                match.emplace_back(
                    rt::fromLen(region->beg[i]),
                    str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
            }
        });
        return match;
    }
    /*!
     * @brief Получить всю информацию о всех найденных вхождениях.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<std::vector<std::pair<size_t, simple_str<K>>>> - массив со всеми вхождениями, в котором каждый элемент -
     *      массив, описывающий вхождение, в котором первый элемент описывает всё вхождение, а следующие -
     *      подгруппы вхождения. Описание представляет собой пару, первый элемент которой - позиция начала вхождения,
     *      второй - текст вхождения.
     */
    SIMREX_API std::vector<std::vector<std::pair<size_t, str_type>>> all_matches(str_type text, size_t offset = 0, size_t maxCount = -1) const;
    /*!
     * @brief Получить всю информацию о всех найденных вхождениях.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<std::vector<std::pair<size_t, T>>> - массив со всеми вхождениями, в котором каждый элемент -
     *      массив, описывающий вхождение, в котором первый элемент описывает всё вхождение, а следующие -
     *      подгруппы вхождения. Описание представляет собой пару, первый элемент которой - позиция начала вхождения,
     *      второй - текст вхождения.
     */
    template<StrType<K> T>
    std::vector<std::vector<std::pair<size_t, T>>> all_matches(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<std::vector<std::pair<size_t, T>>> matches;
        for_all_match(text, offset, maxCount, &matches, [](OnigRegion* region, const OnigUChar* start, void* res){
            std::vector<std::vector<std::pair<size_t, T>>>& matches = *static_cast<std::vector<std::vector<std::pair<size_t, T>>>*>(res);
            auto& match = matches.emplace_back();
            match.reserve(region->num_regs);
            for (int i = 0; i < region->num_regs; i++) {
                match.emplace_back(
                    rt::fromLen(region->beg[i]),
                    str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
            }
        });
        return matches;
    }

    /*!
     * @brief Заменить вхождения на заданный текст.
     * @tparam U - тип исходного текста, выводится из аргумента.
     * @tparam T - тип результата. По умолчанию имеет тип исходного текста, если исходный тип - владеющий (sstring, lstring).
     * @param text - исходный текст, в котором ищем.
     * @param replText - текст, которым заменять найденные вхождения.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @param substGroups - обрабатывать в тексте замены шаблон вставки подгрупп.
     * @return текст, полученный из исходного текста заменой найденных вхождений на заданный шаблон.
     *      При substGroups == true, в шаблоне помимо постоянного текста можно указывать номера подгрупп, вместо них будут
     *      вставлены найденные подгруппы:
     *      $N - подгруппы от 0 до 9 ($1, $2 и т.п.).
     *      ${NNN} - подгруппы от 0 до N (${10}, ${12} и т.п.).
     *      $$ - вставляет один $.
     *      $Любые другие варианты - вставляются как есть.
     */
    template<StrType<K> U, typename T = std::remove_cvref_t<U>> requires storable_str<T, K>
    T replace(U&& text, str_type replText, size_t offset = 0, size_t maxCount = -1, bool substGroups = true) {
        std::optional<T> result;
        do_replace(text, replText, offset, maxCount, substGroups, &result, [](const std::vector<str_type>& parts, void* res) {
            std::optional<T>& result = *static_cast<std::optional<T>*>(res);
            result = expr_join<K, std::vector<str_type>, 0, false, false>{parts, nullptr};
        });
        if (!result) {
            return text;
        }
        return std::move(result).value();
    }
    /*!
     * @brief Заменить вхождения на текст, возвращаемый из функции обработчика.
     * @tparam U - тип исходного текста, выводится из аргумента.
     * @tparam T - тип результата. По умолчанию имеет тип исходного текста, если исходный тип - владеющий (sstring, lstring).
     * @param text - исходный текст, в котором ищем.
     * @param replacer - функция, получающая информацию о вхождении и возвращающая текст, которым будет заменено вхождение.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return текст, полученный из исходного текста заменой найденных вхождений на текст, возвращаемый функцией обработчиком.
     * @details функция обработчик получает информацию о вхождении в виде вектора пар, первый элемент вектора - описывает всё
     *      вхождение, последующие - подгруппы вхождения. Каждая пара содержит позицию начала текста, и сам найденный текст.
     *      Вернуть же она должна текст, который будет вставлен вместо вхождения.
     */
    template<StrType<K> U, typename T = std::remove_cvref_t<U>> requires storable_str<T, K>
    T replace_cb(U&& text, auto replacer, size_t offset = 0, size_t maxCount = -1) {
        str_type from = text;
        auto matches = all_matches(from, offset, maxCount);
        if (matches.empty()) {
            return text;
        }
        std::vector<str_type> parts;
        using replacer_ret_t = decltype(replacer(std::declval<std::vector<std::pair<size_t, str_type>>>()));
        std::vector<replacer_ret_t> calcParts;
        parts.reserve(matches.size() * 2 + 1);
        calcParts.reserve(matches.size());
        size_t last = 0;
        for (const auto& match: matches) {
            size_t delta = match[0].first - last;
            if (delta) {
                parts.emplace_back(from(last, delta));
            }
            parts.emplace_back(calcParts.emplace_back(replacer(match)));
            last = match[0].first + match[0].second.len;
        }
        if (last < from.len) {
            parts.emplace_back(from(last));
        }
        return expr_join<K, std::vector<str_type>, 0, false, false>{parts, nullptr};
    }
protected:
    using repl_result_func = void(*)(const std::vector<str_type>&, void* result);
    SIMREX_API str_type first_founded_str(str_type text, size_t offset) const;
    SIMREX_API void all_founded_str(str_type text, size_t offset, size_t maxCount, void* result, void(*func)(str_type, void*)) const;
    SIMREX_API void for_first_match(str_type text, size_t offset, void* res, void(*func)(OnigRegion*, const OnigUChar*, void*)) const;
    SIMREX_API void for_all_match(str_type text, size_t offset, size_t maxCount, void* res, void(*func)(OnigRegion*, const OnigUChar*, void*)) const;
    SIMREX_API void do_replace(str_type text, str_type replText, size_t offset, size_t maxCount, bool substGroups, void* res, repl_result_func func);
    SIMREX_API static OnigEncoding rex_encoding();
};

using OnigRex = OnigRegexp<u8s>;
using OnigRexW = OnigRegexp<uws>;
using OnigRexU = OnigRegexp<u16s>;
using OnigRexUU = OnigRegexp<u32s>;

} // namespace simrex
