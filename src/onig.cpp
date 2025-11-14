#include <simrex/onig.h>

namespace simrex {

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
void OnigRegexp<K>::all_founded_str(str_type text, size_t offset, size_t maxCount, void(*func)(str_type, void*), void* result) const {
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
std::vector<std::pair<int, typename OnigRegexp<K>::str_type>> OnigRegexp<K>::parse_replaces(str_type replText, bool substGroups) {
    std::vector<std::pair<int, str_type>> replaces;
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
                        replaces.emplace_back(-1, str_type{start, size_t(startGroup - start)});
                    replaces.emplace_back(*fnd - '0', simple_str_nt<K>::empty_str);
                    state = 0;
                    start = fnd + 1;
                } else if (*fnd == '$') {
                    replaces.emplace_back(-1, str_type{start, size_t(fnd - start)});
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
                        replaces.emplace_back(-1, str_type{start, size_t(startGroup - start)});
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
                    fnd = start + dollar;
                }
            }
        }
        if (fnd > start) {
            replaces.emplace_back(-1, str_type{start, size_t(fnd - start)});
        }
    }
    return replaces;
}

// Явно инстанцируем шаблоны для этих типов
template class OnigRegexp<u8s>;
template class OnigRegexp<u16s>;
template class OnigRegexp<u32s>;
template class OnigRegexp<wchar_t>;

} // namespace simrex
