/*
* (c) Проект "SimRex", Александр Орефков orefkov@gmail.com
* Регэкспы на oniguruma для SimStr.
*/
#pragma once
#include <simstr/sstring.h>
#include <oniguruma.h>
#include <list>

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

namespace utils {

template<auto delete_>
struct SimpleDeleter {
    void operator()(auto ptr) const {
        if (ptr) {
            delete_(ptr);
        }
    }
};

} // namespace utils

struct OnigRegionDeleter {
    void operator()(OnigRegion* region) const {
        onig_region_free(region, 1);
    }
};

using RegexPtr = std::unique_ptr<OnigRegexType, utils::SimpleDeleter<onig_free>>;
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

    static OnigRegex create_regex(const OnigUChar* pattern, size_t length, OnigEncoding enc) {
        const OnigUChar *end = pattern + length;
        OnigRegex temp = nullptr;
        return ONIG_NORMAL == onig_new(&temp, pattern, end, ONIG_OPTION_DEFAULT, enc, ONIG_SYNTAX_DEFAULT, nullptr) ? temp : nullptr;
    }

    OnigRegExpBase(OnigRegExpBase&& other) noexcept = default;
    ~OnigRegExpBase() = default;
    OnigRegExpBase& operator=(OnigRegExpBase&& other) noexcept = default;

    int search(const OnigUChar* start, size_t length, size_t offset) const {
        const OnigUChar* end = start + length;
        return onig_search(*this, start, end, start + offset, end, nullptr, ONIG_OPTION_NONE);
    }

    RegexPtr regexp_;
};

template<typename K>
OnigEncoding rexEncoding() {
    if constexpr (sizeof(K) == 2) {
        return std::endian::native == std::endian::big ? ONIG_ENCODING_UTF16_BE : ONIG_ENCODING_UTF16_LE;
    }
    if constexpr (sizeof(K) == 4) {
        return std::endian::native == std::endian::big ? ONIG_ENCODING_UTF32_BE : ONIG_ENCODING_UTF32_LE;
    }
    return ONIG_ENCODING_UTF8;
}

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
    OnigRegexp(str_type pattern) : OnigRegExpBase(rt::toChar(pattern.symbols()), rt::toLen(pattern.length()), rexEncoding<K>()) {}

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
    template<typename T = str_type>
    T first_founded(str_type text, size_t offset = 0) const {
        return first_founded_str(text, offset);
    }
    /*!
     * @brief Получить тексты всех найденных вхождений, без разделения на подгруппы.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<T> - вектор с текстами всех найденных вхождений.
     */
    template<typename T = str_type>
    std::vector<T> all_founded(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<T> matches;
        all_founded_str(text, offset, maxCount, [](str_type word, void* res) {
            reinterpret_cast<std::vector<T>*>(res)->emplace_back(word);
        }, &matches);
        return matches;
    }
    /*!
     * @brief Получить текст первого найденного вхождения вместе с текстами подгрупп.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<T> - массив с текстами найденного вхождения - в первом элементе массива возвращает
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    template<typename T = str_type>
    std::vector<T> first_matched(str_type text, size_t offset = 0) const {
        std::vector<T> matches;
        if (isValid()) {
            const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end());
            RegionPtr region{onig_region_new()};
            if (onig_search(*this, start, end, start + rt::toLen(offset), end, region.get(), ONIG_OPTION_NONE) >= 0) {
                matches.reserve(region->num_regs);
                for (int i = 0; i < region->num_regs; i++) {
                    matches.emplace_back(str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
                }
            }
        }
        return matches;
    }
    /*!
     * @brief Получить тексты всех найденных вхождений вместе с подгруппами.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @param maxCount - максимальное количество для ограничения поиска.
     * @return std::vector<std::vector<T>> - массив с массивами, в которых в первом элементе находится
     *      текст всего найденного вхождения, а далее тексты подгрупп.
     */
    template<typename T = str_type>
    std::vector<std::vector<T>> all_matched(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<std::vector<T>> matches;
        if (isValid()) {
            const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
            RegionPtr region{onig_region_new()};
            for (size_t count = 0; count < maxCount; count++) {
                if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                    auto& match = matches.emplace_back();
                    match.reserve(region->num_regs);
                    for (int i = 0; i < region->num_regs; i++) {
                        match.emplace_back(str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
                    }
                    const OnigUChar* newAt = start + region->end[0];
                    if (newAt <= at || newAt >= end) {
                        break;
                    }
                    at = newAt;
                } else {
                    break;
                }
            }
        }
        return matches;
    }
    // Возвращает массив пар - индекс начала совпадения в строке и его текст + подгруппы

    /*!
     * @brief Получить всю информацию о первом найденном вхождении.
     * @tparam T - тип текста в возвращаемом результате, по умолчанию simple_str<K>.
     * @param text - текст, в котором ищем.
     * @param offset -  начальная позиция поиска (по умолчанию 0).
     * @return std::vector<std::pair<size_t, T>> - массив, в котором первый элемент описывает всё вхождение, а следующие -
     *      подгруппы вхождения. Описание представляет собой пару, первый элемент которой - позиция начала вхождения,
     *      второй - текст вхождения.
     */
    template<typename T = str_type>
    std::vector<std::pair<size_t, T>> first_match(str_type text, size_t offset = 0) const {
        std::vector<std::pair<size_t, T>> matches;
        if (isValid()) {
            const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end());
            RegionPtr region{onig_region_new()};
            int result = onig_search(*this, start, end, start + rt::toLen(offset), end, region.get(), ONIG_OPTION_NONE);
            if (result >= 0) {
                matches.reserve(region->num_regs);
                for (int i = 0; i < region->num_regs; i++) {
                    matches.emplace_back(
                        rt::fromLen(region->beg[i]),
                        str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
                }
            }
        }
        return matches;
    }
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
    template<typename T = str_type>
    std::vector<std::vector<std::pair<size_t, T>>> all_matches(str_type text, size_t offset = 0, size_t maxCount = -1) const {
        std::vector<std::vector<std::pair<size_t, T>>> matches;
        if (isValid()) {
            const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
            RegionPtr region{onig_region_new()};
            for (size_t count = 0; count < maxCount; count++) {
                if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                    auto& match = matches.emplace_back();
                    match.reserve(region->num_regs);
                    for (int i = 0; i < region->num_regs; i++) {
                        match.emplace_back(
                            rt::fromLen(region->beg[i]),
                            str_type{rt::fromChar(start + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])});
                    }
                    const OnigUChar* newAt = start + region->end[0];
                    if (newAt <= at || newAt >= end) {
                        break;
                    }
                    at = newAt;
                } else {
                    break;
                }
            }
        }
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
        if (!regexp_) {
            return text;
        }
        auto replaces = parse_replaces(replText, substGroups);

        std::vector<str_type> parts;
        size_t delta = 0;
        const OnigUChar *starto = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = starto + rt::toLen(offset),
                        *prevStart = starto;
        RegionPtr region{onig_region_new()};
        for (size_t count = 0; count < maxCount; count++) {
            int result = onig_search(*this, starto, end, at, end, region.get(), ONIG_OPTION_NONE);
            if (result >= 0) {
                delta = rt::fromLen(int(starto + region->beg[0] - prevStart));
                if (delta) {
                    parts.emplace_back(rt::fromChar(prevStart), delta);
                }
                for (const auto& [idx, text]: replaces) {
                    if (idx < 0) {
                        parts.emplace_back(text);
                    } else if (idx < region->num_regs) {
                        delta = rt::fromLen(region->end[idx] - region->beg[idx]);
                        if (delta) {
                            parts.emplace_back(rt::fromChar(starto + region->beg[idx]), delta);
                        }
                    }
                }
                const OnigUChar* newAt = starto + region->end[0];
                if (newAt <= at || at >= end) {
                    break;
                }
                at = prevStart = newAt;
            } else {
                break;
            }
        }
        if (parts.empty()) {
            return std::forward<U>(text);
        }
        if (at < end) {
            parts.emplace_back(rt::fromChar(at), rt::fromLen(int(end - at)));
        }
        return expr_join<K, std::vector<str_type>, 0, false, false>{parts, nullptr};
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
        if (!regexp_) {
            return text;
        }
        std::vector<str_type> parts;
        using replacer_ret_t = decltype(replacer(std::declval<std::vector<std::pair<size_t, str_type>>>()));
        std::list<replacer_ret_t> calcParts;
        
        size_t delta = 0;
        const OnigUChar *starto = rt::toChar(text.symbols()), *end = rt::toChar(text.symbols() + text.length()), *at = starto + rt::toLen(offset),
                        *prevStart = starto;
        RegionPtr region{onig_region_new()};
        
        for (size_t count = 0; count < maxCount; count++) {
            int result = onig_search(*this, starto, end, at, end, region.get(), ONIG_OPTION_NONE);
            if (result >= 0) {
                delta = rt::fromLen(int(starto + region->beg[0] - prevStart));
                if (delta) {
                    parts.emplace_back(rt::fromChar(prevStart), delta);
                }
                std::vector<std::pair<size_t, str_type>> match;
                match.reserve(region->num_regs);
                for (int i = 0; i < region->num_regs; i++) {
                    match.emplace_back(
                        rt::fromLen(region->beg[i]),
                        str_type{rt::fromChar(starto + region->beg[i]), rt::fromLen(region->end[i] - region->beg[i])}
                    );
                }

                calcParts.emplace_back(replacer(std::move(match)));
                parts.emplace_back(calcParts.back());

                const OnigUChar* newAt = starto + region->end[0];
                if (newAt <= at || at >= end) {
                    break;
                }
                at = prevStart = newAt;
            } else {
                break;
            }
        }
        if (parts.empty()) {
            // Ничего не заменили, просто форварднем исходный текст.
            return std::forward<U>(text);
        }
        if (at < end) {
            // Добавим последний кусочек
            parts.emplace_back(rt::fromChar(at), rt::fromLen(int(end - at)));
        }
        return expr_join<K, std::vector<str_type>, 0, false, false>{parts, nullptr};
    }
protected:
    SIMREX_API str_type first_founded_str(str_type text, size_t offset) const;
    SIMREX_API void all_founded_str(str_type text, size_t offset, size_t maxCount, void(*func)(str_type, void*), void* result) const;
    SIMREX_API std::vector<std::pair<int, str_type>> parse_replaces(str_type replText, bool substGroups);
};

using OnigRex = OnigRegexp<u8s>;
using OnigRexW = OnigRegexp<uws>;
using OnigRexU = OnigRegexp<u16s>;
using OnigRexUU = OnigRegexp<u32s>;

} // namespace simrex
