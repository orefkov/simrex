#include <simrex/onig.h>

namespace simrex {

void OnigRegionDeleter::operator()(OnigRegion* region) const {
    onig_region_free(region, 1);
}

void  OnigRexDeleter::operator()(OnigRegex rex) const {
    onig_free(rex);
}

OnigRegex OnigRegExpBase::create_regex(const OnigUChar* pattern, size_t length, OnigEncoding enc) {
    const OnigUChar *end = pattern + length;
    OnigRegex temp = nullptr;
    return ONIG_NORMAL == onig_new(&temp, pattern, end, ONIG_OPTION_DEFAULT, enc, ONIG_SYNTAX_DEFAULT, nullptr) ? temp : nullptr;
}

int OnigRegExpBase::search(const OnigUChar* start, size_t length, size_t offset) const  {
    const OnigUChar* end = start + length;
    return onig_search(*this, start, end, start + offset, end, nullptr, ONIG_OPTION_NONE);
}

template<typename K>
size_t OnigRegexp<K>::count_of(const str_type& text, size_t maxCount, size_t offset) {
    size_t matches = 0;
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
        RegionPtr region{onig_region_new()};
        for (size_t count = 0; count < maxCount; count++) {
            if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                matches++;
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

template<typename K>
typename OnigRegexp<K>::str_type OnigRegexp<K>::first_founded_str(str_type text, size_t offset) const {
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end());
        RegionPtr region{onig_region_new()};
        if (onig_search(*this, start, end, start + rt::toLen(offset), end, region.get(), ONIG_OPTION_NONE) >= 0) {
            return str_type{rt::fromChar(start + region->beg[0]), rt::fromLen(region->end[0] - region->beg[0])};
        }
    }
    return simple_str_nt<K>::empty_str;
}

template<typename K>
std::vector<typename OnigRegexp<K>::str_type> OnigRegexp<K>::texts_in_first_match(str_type text, size_t offset) const {
    std::vector<str_type> matches;
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

template<typename K>
void OnigRegexp<K>::for_first_match(str_type text, size_t offset, void* res, void(*func)(OnigRegion*, const OnigUChar*, void*)) const {
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end());
        RegionPtr region{onig_region_new()};
        if (onig_search(*this, start, end, start + rt::toLen(offset), end, region.get(), ONIG_OPTION_NONE) >= 0) {
            func(region.get(), start, res);
        }
    }
}

template<typename K>
void OnigRegexp<K>::all_founded_str(str_type text, size_t offset, size_t maxCount, void* result, void(*func)(str_type, void*)) const {
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
        RegionPtr region{onig_region_new()};
        for (size_t count = 0; count < maxCount; count++) {
            if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                func(str_type{rt::fromChar(start + region->beg[0]), rt::fromLen(region->end[0] - region->beg[0])}, result);
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
}

template<typename K>
std::vector<typename OnigRegexp<K>::str_type> OnigRegexp<K>::all_founded(str_type text, size_t offset, size_t maxCount) const {
    std::vector<str_type> matches;
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
        RegionPtr region{onig_region_new()};
        for (size_t count = 0; count < maxCount; count++) {
            if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                matches.emplace_back(rt::fromChar(start + region->beg[0]), rt::fromLen(region->end[0] - region->beg[0]));
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

template<typename K>
std::vector<std::vector<typename OnigRegexp<K>::str_type>> OnigRegexp<K>::texts_in_all_matches(str_type text, size_t offset, size_t maxCount) const {
    std::vector<std::vector<str_type>> matches;
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

template<typename K>
void OnigRegexp<K>::for_all_match(str_type text, size_t offset, size_t maxCount, void* res, void(*func)(OnigRegion*, const OnigUChar*, void*)) const {
    if (isValid()) {
        const OnigUChar *start = rt::toChar(text.begin()), *end = rt::toChar(text.end()), *at = start + rt::toLen(offset);
        RegionPtr region{onig_region_new()};
        for (size_t count = 0; count < maxCount; count++) {
            if (onig_search(*this, start, end, at, end, region.get(), ONIG_OPTION_NONE) >= 0) {
                func(region.get(), start, res);
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
}

template<typename K>
std::vector<std::pair<size_t, typename OnigRegexp<K>::str_type>> OnigRegexp<K>::first_match(str_type text, size_t offset) const {
    std::vector<std::pair<size_t, str_type>> matches;
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

template<typename K>
std::vector<std::vector<std::pair<size_t, typename OnigRegexp<K>::str_type>>> OnigRegexp<K>::all_matches(str_type text, size_t offset, size_t maxCount) const {
    std::vector<std::vector<std::pair<size_t, str_type>>> matches;
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

template<typename K>
std::vector<std::pair<int, simple_str<K>>> parse_replaces(simple_str<K> replText, bool substGroups) {
    std::vector<std::pair<int, simple_str<K>>> replaces;
    size_t dollar = -1;
    if (!substGroups || (dollar = replText.find(K('$'))) == str::npos) {
        replaces.emplace_back(-1, replText);
    } else {
        const K *start = replText.symbols(), *fnd = start + dollar, *startGroup = nullptr, *endRepl = start + replText.length();
        int state = 0, numOfGroup = 0;
        while (fnd < endRepl) {
            switch (state) {
            case 0:
                startGroup = fnd;
                state = 1;
                break;
            case 1:
                if (*fnd == '{') {
                    state = 2;
                } else if (*fnd >= '0' && *fnd <= '9') {
                    if (startGroup > start)
                        replaces.emplace_back(-1, simple_str<K>{start, size_t(startGroup - start)});
                    replaces.emplace_back(*fnd - '0', simple_str_nt<K>::empty_str);
                    state = 0;
                    start = fnd + 1;
                } else if (*fnd == '$') {
                    replaces.emplace_back(-1, simple_str<K>{start, size_t(fnd - start)});
                    state = 0;
                    start = fnd + 1;
                } else {
                    state = 0;
                }
                break;
            case 2:
                if (*fnd >= '0' && *fnd <= '9') {
                    numOfGroup = *fnd - '0';
                    state = 3;
                } else {
                    state = 0;
                }
                break;
            case 3:
                if (*fnd >= '0' && *fnd <= '9') {
                    numOfGroup = numOfGroup * 10 + *fnd - '0';
                } else if (*fnd == '}') {
                    if (startGroup > start)
                        replaces.emplace_back(-1, simple_str<K>{start, size_t(startGroup - start)});
                    replaces.emplace_back(numOfGroup, simple_str_nt<K>::empty_str);
                    state = 0;
                    start = fnd + 1;
                } else {
                    state = 0;
                }
            }
            fnd++;
            if (state == 0) {
                dollar = replText.find(K('$'), fnd - replText.str);
                if (dollar == str::npos) {
                    fnd = endRepl;
                } else {
                    fnd = replText.str + dollar;
                }
            }
        }
        if (fnd > start) {
            replaces.emplace_back(-1, simple_str<K>{start, size_t(fnd - start)});
        }
    }
    return replaces;
}

template<typename K>
void OnigRegexp<K>::do_replace(str_type text, str_type replText, size_t offset, size_t maxCount, bool substGroups, void* res, repl_result_func func) {
    if (!regexp_) {
        return;
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
    if (!parts.empty()) {
        if (at < end) {
            parts.emplace_back(rt::fromChar(at), rt::fromLen(int(end - at)));
        }
        func(parts, res);
    }
}
template<typename K>
OnigEncoding OnigRegexp<K>::rex_encoding() {
    if constexpr (sizeof(K) == 2) {
        return std::endian::native == std::endian::big ? ONIG_ENCODING_UTF16_BE : ONIG_ENCODING_UTF16_LE;
    }
    if constexpr (sizeof(K) == 4) {
        return std::endian::native == std::endian::big ? ONIG_ENCODING_UTF32_BE : ONIG_ENCODING_UTF32_LE;
    }
    return ONIG_ENCODING_UTF8;
}

// Явно инстанцируем шаблоны для этих типов
template class OnigRegexp<u8s>;
template class OnigRegexp<u16s>;
template class OnigRegexp<u32s>;
template class OnigRegexp<wchar_t>;

} // namespace simrex
