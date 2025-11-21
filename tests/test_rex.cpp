#include <simrex/onig.h>
#define re_registers posix_re_registers
#include <gtest/gtest.h>
namespace simrex::testing {

TEST(SimRex, CreateRex) {
    OnigRegexp<u8s> rex8;
    EXPECT_FALSE(rex8.isValid());

    rex8 = {".?"};
    EXPECT_TRUE(rex8.isValid());

    OnigRegexp<u16s> rex16;
    EXPECT_FALSE(rex16.isValid());

    rex16 = {u".?"};
    EXPECT_TRUE(rex16.isValid());

    OnigRegexp<u32s> rex32;
    EXPECT_FALSE(rex32.isValid());

    rex32 = {U".?"};
    EXPECT_TRUE(rex32.isValid());
}

TEST(SimRex, Search) {
    OnigRegexp<u8s> rex8{"a+"};
    EXPECT_TRUE(rex8.isValid());
    EXPECT_EQ(rex8.search("bbbbaa"), 4);
    EXPECT_EQ(rex8.search("bbbbaa", 5), 5);
    EXPECT_EQ(rex8.search("bbbbaa", 10), -1);
    EXPECT_EQ(rex8.search("bbbb"), -1);

    OnigRegexp<u16s> rex16{u"a+"};
    EXPECT_TRUE(rex16.isValid());
    EXPECT_EQ(rex16.search(u"bbbbaa", 5), 5);
    EXPECT_EQ(rex16.search(u"bbbbaa", 10), -1);
    EXPECT_EQ(rex16.search(u"bbbbaa"), 4);
    EXPECT_EQ(rex16.search(u"bbbb"), -1);

    OnigRegexp<u32s> rex32{U"a+"};
    EXPECT_TRUE(rex32.isValid());
    EXPECT_EQ(rex32.search(U"bbbbaa", 5), 5);
    EXPECT_EQ(rex32.search(U"bbbbaa", 10), -1);
    EXPECT_EQ(rex32.search(U"bbbbaa"), 4);
    EXPECT_EQ(rex32.search(U"bbbb"), -1);
}
TEST(SimRex, FindedFirst) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        EXPECT_EQ(rex.first_founded("bbbaabbbabbaaa"), "baa");
        EXPECT_EQ(rex.first_founded("bbbaabbbabbaaa", 3), "ba");
        EXPECT_EQ(rex.first_founded("cccccc"), "");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        EXPECT_EQ(rex.first_founded(u"bbbaabbbabbaaa"), u"baa");
        EXPECT_EQ(rex.first_founded(u"bbbaabbbabbaaa", 3), u"ba");
        EXPECT_EQ(rex.first_founded(u"cccccc"), u"");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        EXPECT_EQ(rex.first_founded(U"bbbaabbbabbaaa"), U"baa");
        EXPECT_EQ(rex.first_founded(U"bbbaabbbabbaaa", 3), U"ba");
        EXPECT_EQ(rex.first_founded(U"cccccc"), U"");
    }
    {
        OnigRegexp<uws> rex{L"b(a+)"};
        EXPECT_EQ(rex.first_founded(L"bbbaabbbabbaaa"), L"baa");
        EXPECT_EQ(rex.first_founded(L"bbbaabbbabbaaa", 3), L"ba");
        EXPECT_EQ(rex.first_founded(L"cccccc"), L"");
    }
}
TEST(SimRex, FindedAll) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        auto match = rex.all_founded("bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3u);
        EXPECT_EQ(match[0], "baa");
        EXPECT_EQ(match[1], "ba");
        EXPECT_EQ(match[2], "baaa");
        match = rex.all_founded("bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], "ba");
        EXPECT_EQ(match[1], "baaa");
        match = rex.all_founded("bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], "baa");
        EXPECT_EQ(match[1], "ba");
        match = rex.all_founded("cccccc", 1, 2);
        EXPECT_EQ(match.size(), 0u);
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        auto match = rex.all_founded(u"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3u);
        EXPECT_EQ(match[0], u"baa");
        EXPECT_EQ(match[1], u"ba");
        EXPECT_EQ(match[2], u"baaa");
        match = rex.all_founded(u"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], u"ba");
        EXPECT_EQ(match[1], u"baaa");
        match = rex.all_founded(u"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], u"baa");
        EXPECT_EQ(match[1], u"ba");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        auto match = rex.all_founded(U"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3u);
        EXPECT_EQ(match[0], U"baa");
        EXPECT_EQ(match[1], U"ba");
        EXPECT_EQ(match[2], U"baaa");
        match = rex.all_founded(U"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], U"ba");
        EXPECT_EQ(match[1], U"baaa");
        match = rex.all_founded(U"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], U"baa");
        EXPECT_EQ(match[1], U"ba");
    }
}

TEST(SimRex, MatchFirstSimple) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.texts_in_first_match("bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], "baa");
        EXPECT_EQ(match[1], "aa");
        match = rex.texts_in_first_match("bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], "ba");
        EXPECT_EQ(match[1], "a");

        auto smatch = rex.texts_in_first_match<stringa>("bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0], "baa");
        EXPECT_EQ(smatch[1], "aa");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.texts_in_first_match(u"bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], u"baa");
        EXPECT_EQ(match[1], u"aa");
        match = rex.texts_in_first_match(u"bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], u"ba");
        EXPECT_EQ(match[1], u"a");
        auto smatch = rex.texts_in_first_match<stringu>(u"bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0], u"baa");
        EXPECT_EQ(smatch[1], u"aa");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.texts_in_first_match(U"bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], U"baa");
        EXPECT_EQ(match[1], U"aa");
        match = rex.texts_in_first_match(U"bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0], U"ba");
        EXPECT_EQ(match[1], U"a");
        auto smatch = rex.texts_in_first_match<stringuu>(U"bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0], U"baa");
        EXPECT_EQ(smatch[1], U"aa");
    }
}

TEST(SimRex, MatchFirst) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.first_match("bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 3);
        EXPECT_EQ(match[0].second, "baa");
        EXPECT_EQ(match[1].first, 4);
        EXPECT_EQ(match[1].second, "aa");
        match = rex.first_match("bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 6);
        EXPECT_EQ(match[0].second, "ba");
        EXPECT_EQ(match[1].first, 7);
        EXPECT_EQ(match[1].second, "a");

        auto smatch = rex.first_match<stringa>("bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0].first, 3);
        EXPECT_EQ(smatch[0].second, "baa");
        EXPECT_EQ(smatch[1].first, 4);
        EXPECT_EQ(smatch[1].second, "aa");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.first_match(u"bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 3);
        EXPECT_EQ(match[0].second, u"baa");
        EXPECT_EQ(match[1].first, 4);
        EXPECT_EQ(match[1].second, u"aa");
        match = rex.first_match(u"bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 6);
        EXPECT_EQ(match[0].second, u"ba");
        EXPECT_EQ(match[1].first, 7);
        EXPECT_EQ(match[1].second, u"a");

        auto smatch = rex.first_match<stringu>(u"bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0].first, 3);
        EXPECT_EQ(smatch[0].second, u"baa");
        EXPECT_EQ(smatch[1].first, 4);
        EXPECT_EQ(smatch[1].second, u"aa");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        EXPECT_TRUE(rex.isValid());
        auto match = rex.first_match(U"bbbbaaba");
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 3);
        EXPECT_EQ(match[0].second, U"baa");
        EXPECT_EQ(match[1].first, 4);
        EXPECT_EQ(match[1].second, U"aa");
        match = rex.first_match(U"bbbbaaba", 4);
        EXPECT_EQ(match.size(), 2u);
        EXPECT_EQ(match[0].first, 6);
        EXPECT_EQ(match[0].second, U"ba");
        EXPECT_EQ(match[1].first, 7);
        EXPECT_EQ(match[1].second, U"a");

        auto smatch = rex.first_match<stringuu>(U"bbbbaa");
        EXPECT_EQ(smatch.size(), 2u);
        EXPECT_EQ(smatch[0].first, 3);
        EXPECT_EQ(smatch[0].second, U"baa");
        EXPECT_EQ(smatch[1].first, 4);
        EXPECT_EQ(smatch[1].second, U"aa");
    }
}

TEST(SimRex, MatchAllSimple) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        auto match = rex.texts_in_all_matches("bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0], "baa");
        EXPECT_EQ(match[0][1], "aa");
        EXPECT_EQ(match[1][0], "ba");
        EXPECT_EQ(match[1][1], "a");
        EXPECT_EQ(match[2][0], "baaa");
        EXPECT_EQ(match[2][1], "aaa");

        match = rex.texts_in_all_matches("bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], "ba");
        EXPECT_EQ(match[0][1], "a");
        EXPECT_EQ(match[1][0], "baaa");
        EXPECT_EQ(match[1][1], "aaa");

        match = rex.texts_in_all_matches("bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], "baa");
        EXPECT_EQ(match[0][1], "aa");
        EXPECT_EQ(match[1][0], "ba");
        EXPECT_EQ(match[1][1], "a");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        auto match = rex.texts_in_all_matches(u"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0], u"baa");
        EXPECT_EQ(match[0][1], u"aa");
        EXPECT_EQ(match[1][0], u"ba");
        EXPECT_EQ(match[1][1], u"a");
        EXPECT_EQ(match[2][0], u"baaa");
        EXPECT_EQ(match[2][1], u"aaa");

        match = rex.texts_in_all_matches(u"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], u"ba");
        EXPECT_EQ(match[0][1], u"a");
        EXPECT_EQ(match[1][0], u"baaa");
        EXPECT_EQ(match[1][1], u"aaa");

        match = rex.texts_in_all_matches(u"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], u"baa");
        EXPECT_EQ(match[0][1], u"aa");
        EXPECT_EQ(match[1][0], u"ba");
        EXPECT_EQ(match[1][1], u"a");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        auto match = rex.texts_in_all_matches(U"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0], U"baa");
        EXPECT_EQ(match[0][1], U"aa");
        EXPECT_EQ(match[1][0], U"ba");
        EXPECT_EQ(match[1][1], U"a");
        EXPECT_EQ(match[2][0], U"baaa");
        EXPECT_EQ(match[2][1], U"aaa");

        match = rex.texts_in_all_matches(U"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], U"ba");
        EXPECT_EQ(match[0][1], U"a");
        EXPECT_EQ(match[1][0], U"baaa");
        EXPECT_EQ(match[1][1], U"aaa");

        match = rex.texts_in_all_matches(U"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0], U"baa");
        EXPECT_EQ(match[0][1], U"aa");
        EXPECT_EQ(match[1][0], U"ba");
        EXPECT_EQ(match[1][1], U"a");
    }
}

TEST(SimRex, MatchAll) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        auto match = rex.all_matches("bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0].first, 2);
        EXPECT_EQ(match[0][0].second, "baa");
        EXPECT_EQ(match[0][1].first, 3);
        EXPECT_EQ(match[0][1].second, "aa");
        EXPECT_EQ(match[1][0].first, 7);
        EXPECT_EQ(match[1][0].second, "ba");
        EXPECT_EQ(match[1][1].first, 8);
        EXPECT_EQ(match[1][1].second, "a");
        EXPECT_EQ(match[2][0].second, "baaa");
        EXPECT_EQ(match[2][1].second, "aaa");

        match = rex.all_matches("bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, "ba");
        EXPECT_EQ(match[0][1].second, "a");
        EXPECT_EQ(match[1][0].second, "baaa");
        EXPECT_EQ(match[1][1].second, "aaa");

        match = rex.all_matches("bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, "baa");
        EXPECT_EQ(match[0][1].second, "aa");
        EXPECT_EQ(match[1][0].second, "ba");
        EXPECT_EQ(match[1][1].second, "a");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        auto match = rex.all_matches(u"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0].first, 2);
        EXPECT_EQ(match[0][0].second, u"baa");
        EXPECT_EQ(match[0][1].first, 3);
        EXPECT_EQ(match[0][1].second, u"aa");
        EXPECT_EQ(match[1][0].first, 7);
        EXPECT_EQ(match[1][0].second, u"ba");
        EXPECT_EQ(match[1][1].first, 8);
        EXPECT_EQ(match[1][1].second, u"a");
        EXPECT_EQ(match[2][0].second, u"baaa");
        EXPECT_EQ(match[2][1].second, u"aaa");

        match = rex.all_matches(u"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, u"ba");
        EXPECT_EQ(match[0][1].second, u"a");
        EXPECT_EQ(match[1][0].second, u"baaa");
        EXPECT_EQ(match[1][1].second, u"aaa");

        match = rex.all_matches(u"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, u"baa");
        EXPECT_EQ(match[0][1].second, u"aa");
        EXPECT_EQ(match[1][0].second, u"ba");
        EXPECT_EQ(match[1][1].second, u"a");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        auto match = rex.all_matches(U"bbbaabbbabbaaa");
        EXPECT_EQ(match.size(), 3);
        EXPECT_EQ(match[0][0].first, 2);
        EXPECT_EQ(match[0][0].second, U"baa");
        EXPECT_EQ(match[0][1].first, 3);
        EXPECT_EQ(match[0][1].second, U"aa");
        EXPECT_EQ(match[1][0].first, 7);
        EXPECT_EQ(match[1][0].second, U"ba");
        EXPECT_EQ(match[1][1].first, 8);
        EXPECT_EQ(match[1][1].second, U"a");
        EXPECT_EQ(match[2][0].second, U"baaa");
        EXPECT_EQ(match[2][1].second, U"aaa");

        match = rex.all_matches(U"bbbaabbbabbaaa", 4);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, U"ba");
        EXPECT_EQ(match[0][1].second, U"a");
        EXPECT_EQ(match[1][0].second, U"baaa");
        EXPECT_EQ(match[1][1].second, U"aaa");

        match = rex.all_matches(U"bbbaabbbabbaaa", 1, 2);
        EXPECT_EQ(match.size(), 2);
        EXPECT_EQ(match[0][0].second, U"baa");
        EXPECT_EQ(match[0][1].second, U"aa");
        EXPECT_EQ(match[1][0].second, U"ba");
        EXPECT_EQ(match[1][1].second, U"a");
    }
}

TEST(SimRex, Replace) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        EXPECT_EQ(rex.replace<stringa>("bbbaabbbabbaaa", "-"), "bb-bb-b-");
        EXPECT_EQ(rex.replace<stringa>("bbbaabbbabbaaa", "-$1^"), "bb-aa^bb-a^b-aaa^");
        EXPECT_EQ(rex.replace<stringa>("bbbaabbbabbaaa", "-$1^", 1, 2), "bb-aa^bb-a^bbaaa");
    }
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        auto res = rex.replace(stringa{"bbbaabbbabbaaa"}, "-"); 
        EXPECT_EQ(res, "bb-bb-b-");
        auto res1 = rex.replace<lstringa<40>>(stringa{"bbbaabbbabbaaa"}, "-"); 
        EXPECT_EQ(res1, "bb-bb-b-");
    }
    {
        OnigRegexp<u16s> rex{u"b(a+)"};
        EXPECT_EQ(rex.replace<stringu>(u"bbbaabbbabbaaa", u"-"), u"bb-bb-b-");
        EXPECT_EQ(rex.replace<stringu>(u"bbbaabbbabbaaa", u"-$1^"), u"bb-aa^bb-a^b-aaa^");
        EXPECT_EQ(rex.replace<stringu>(u"bbbaabbbabbaaa", u"-$1^", 1, 2), u"bb-aa^bb-a^bbaaa");
    }
    {
        OnigRegexp<u32s> rex{U"b(a+)"};
        EXPECT_EQ(rex.replace<stringuu>(U"bbbaabbbabbaaa", U"-"), U"bb-bb-b-");
        EXPECT_EQ(rex.replace<stringuu>(U"bbbaabbbabbaaa", U"-$1^"), U"bb-aa^bb-a^b-aaa^");
        EXPECT_EQ(rex.replace<stringuu>(U"bbbaabbbabbaaa", U"-$1^", 1, 2), U"bb-aa^bb-a^bbaaa");
    }
    {
        OnigRegexp<u8s> rex{"d"};
        stringa v = "bbbaabbbabbaaa";
        stringa r = rex.replace(v, "-");
        EXPECT_EQ(v, "bbbaabbbabbaaa");
        EXPECT_EQ(v, r);
        // Так как замены не было, строка-результат должна была инициироваться из исходной строки, и
        // значит просто скопировать ссылку на литерал.
        EXPECT_EQ(v.c_str(), r.c_str());
    }
    {
        OnigRegexp<u8s> rex{"b"};
        stringa v = "bbbaabbbabbaaa";
        stringa r = rex.replace(v, "$0");
        // Вхождение заменялось на само себя
        EXPECT_EQ(v, "bbbaabbbabbaaa");
        EXPECT_EQ(v, r);
        // Так как замена была, строка-результат должна стать новой, хотя и с тем же содержанием.
        EXPECT_NE(v.c_str(), r.c_str());
    }
}

TEST(SimRex, ReplaceCb) {
    {
        OnigRegexp<u8s> rex{"b(a+)"};
        EXPECT_EQ(rex.replace_cb<stringa>("bbbaabbbabbaaa", [](const std::vector<std::pair<size_t, ssa>>& match) -> stringa {
            return "-" + match[1].second + "-";
        }), "bb-aa-bb-a-b-aaa-");
    }
    {
        OnigRegexp<u8s> rex{"d"};
        stringa v = "bbbaabbbabbaaa";
        stringa r = rex.replace_cb(v, [](const auto&){return stringa{};});
        EXPECT_EQ(v, "bbbaabbbabbaaa");
        EXPECT_EQ(v, r);
        // Так как замены не было, строка-результат должна была инициироваться из исходной строки, и
        // значит просто скопировать ссылку на литерал.
        EXPECT_EQ(v.c_str(), r.c_str());
    }
    {
        OnigRegexp<u8s> rex{"b"};
        stringa v = "bbbaabbbabbaaa";
        stringa r = rex.replace_cb(v, [](const auto& match){return stringa{match[0].second};});
        EXPECT_EQ(v, "bbbaabbbabbaaa");
        EXPECT_EQ(v, r);
        // Так как замена была, строка-результат должна стать новой, хотя и с тем же содержанием.
        EXPECT_NE(v.c_str(), r.c_str());
    }
}
} // namespace simrex::testing
