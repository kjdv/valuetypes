#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

namespace valuetypes {

// TBD: how to include minijson in generated code? As separate library (complicating integrations into target source), or
//      part of generated code (duplication)
//      For now: dirty and practical: develop here and copy-paste into generated source

namespace minijson {

class BadJson : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

// tokens
struct start_mapping_token {};
struct end_mapping_token {};
struct start_sequence_token {};
struct end_sequence_token {};
struct separator_token {};
struct mapper_token {};
struct string_token {
    std::string value;
};
struct int_token {
    std::string value;
};
struct float_token {
    std::string value;
};
struct true_token {};
struct false_token {};
struct null_token {};
struct eof_token {};

using token = std::variant<
    start_mapping_token,
    end_mapping_token,
    start_sequence_token,
    end_sequence_token,
    separator_token,
    mapper_token,
    string_token,
    int_token,
    float_token,
    true_token,
    false_token,
    null_token,
    eof_token>;

template <typename T>
bool match_token(const token& t) noexcept {
    return std::holds_alternative<T>(t);
}

template <typename T>
void check_token(const token& t) {
    if(!match_token<T>(t)) {
        throw BadJson("unexpected token");
    }
}

const int eof = std::char_traits<char>::eof();

int non_ws(std::istream& str) noexcept {
    char c = 0;
    while(str.get(c)) {
        if(!isspace(c))
            return c;
    }
    return eof;
}

void extract_literal(std::istream& input, std::string_view tail) {
    for(char e : tail) {
        int c = input.get();
        if(c != e)
            throw BadJson("unexpected char in literal");
    }
}

token extract_number(std::istream& input, char head) noexcept {
    bool is_float  = false;
    bool had_point = false;
    bool had_exp   = false;

    std::string value;
    value += head;

    int c;
    while((c = input.peek()) != eof) {
        if(c >= '0' && c <= '9') {
            input.get();
            value += c;
        } else if(!had_point && c == '.') {
            input.get();
            value += c;
            is_float  = true;
            had_point = true;
        } else if(!had_exp && (c == 'e' || c == 'E')) {
            input.get();
            value += c;

            if(input.peek() == '+' || input.peek() == '-') {
                c = input.get();
                value += c;
            }
            is_float = true;
            had_exp  = true;
        } else
            break;
    }

    if(is_float) {
        return float_token{value};
    } else {
        return int_token{value};
    }
}

bool is_hex(char c) noexcept {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

uint8_t single_decode(char h) noexcept {
    if(h >= '0' && h <= '9')
        return h - '0';
    else if(h >= 'a' && h <= 'f')
        return h - 'a' + 10;
    else if(h >= 'A' && h <= 'F')
        return h - 'A' + 10;

    assert(false);
    return 0;
}

std::string extract_utf8(std::istream& input) {
    char32_t wc = 0;
    for(size_t i = 0; i < 4; ++i) {
        int c = input.get();
        if(c == eof)
            break;

        if(!is_hex(c))
            throw BadJson("expected hex digit");

        wc <<= 4;
        wc |= single_decode((char)c);
    }

    std::string value;

    char byte;

    byte = wc >> 24;
    if(byte)
        value += byte;

    byte = wc >> 16;
    if(byte)
        value += byte;

    byte = wc >> 8;
    if(byte)
        value += byte;

    byte = wc & 0xff;
    value += byte;

    return value;
}

string_token extract_string(std::istream& input) {
    std::string value;

    int c;
    while((c = input.get()) != eof && c != '"') {
        if(c == '\\') {
            c = input.get();
            switch(c) {
            case '/':
            case '\\':
            case '"':
                value += c;
                break;

            case 'b':
                value += '\b';
                break;
            case 'f':
                value += '\f';
                break;
            case 'n':
                value += '\n';
                break;
            case 'r':
                value += '\r';
                break;
            case 't':
                value += '\t';
                break;

            case 'u':
                value += extract_utf8(input);
                break;

            default:
                value += c;
                break;
            }
        } else
            value += c;
    }

    return string_token{value};
}

bool peek(std::istream& stream, char expect) {
    while(isspace(stream.peek())) {
        stream.get();
    }
    return stream.peek() == expect;
}

token next_token(std::istream& input) {
    int c = non_ws(input);
    if(c != eof) {
        switch(c) {
        case '{':
            return start_mapping_token{};
        case '}':
            return end_mapping_token{};
        case '[':
            return start_sequence_token{};
        case ']':
            return end_sequence_token{};
        case ',':
            return separator_token{};
        case ':':
            return mapper_token{};

        case 't':
            extract_literal(input, "rue");
            return true_token{};
        case 'f':
            extract_literal(input, "alse");
            return false_token{};
        case 'n':
            extract_literal(input, "ull");
            return null_token{};

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '+':
            return extract_number(input, c);

        case '"':
            return extract_string(input);

        default:
            throw BadJson("unexpected token");
        }
    }
    return eof_token{};
}

template <typename T>
void match_and_consume(std::istream& stream) {
    auto t = next_token(stream);
    check_token<T>(t);
}

template <typename T, typename F>
void read_kvpair(std::istream& stream, token current, T& target, F&& consumer) {
    check_token<string_token>(current);
    std::string_view key = std::get<string_token>(current).value;

    match_and_consume<mapper_token>(stream);
    consumer(stream, target, key);
}

template <typename T, typename F>
void read_members(std::istream& stream, T& target, F&& consumer) {
    while(true) {
        auto tok = next_token(stream);
        if(match_token<string_token>(tok)) {
            read_kvpair(stream, tok, target, std::forward<F>(consumer));
            tok = next_token(stream);
        }

        if(match_token<end_mapping_token>(tok)) {
            return;
        } else if(!match_token<separator_token>(tok)) {
            throw BadJson("unexpected token");
        }
    }
}

template <typename T, typename F>
void read_elements(std::istream& stream, T& target, F&& consumer) {
    while(!peek(stream, ']')) {
        consumer(stream, target);
        auto tok = next_token(stream);
        if(match_token<end_sequence_token>(tok)) {
            return;
        }
        check_token<separator_token>(tok);
    }
}

template <typename T, typename F> // signature of F is expected to be void(std::istream& stream, T& target, std::string_view key);
void read_mapping(std::istream& stream, T& target, F&& consumer) {
    match_and_consume<start_mapping_token>(stream);
    read_members(stream, target, std::forward<F>(consumer));
}

template <typename T, typename F> // signature of F is expected to be void(std::istream& stream, T& target);
void read_sequence(std::istream& stream, T& target, F&& consumer) {
    match_and_consume<start_sequence_token>(stream);
    read_elements(stream, target, std::forward<F>(consumer));
}

template <typename T, typename Tok>
T extract(const token& tok) {
    std::istringstream istream(std::get<Tok>(tok).value);
    T                  target;
    istream >> target;
    return target;
}

template <typename T>
std::enable_if_t<std::is_same_v<bool, T>, T> read_actual_scalar(token tok) {
    if(match_token<true_token>(tok)) {
        return true;
    } else if(match_token<false_token>(tok)) {
        return false;
    } else {
        throw BadJson("expected either 'true' or 'false");
    }
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> read_actual_scalar(token tok) {
    if(match_token<float_token>(tok)) {
        return extract<T, float_token>(tok);
    } else if(match_token<int_token>(tok)) {
        return extract<T, int_token>(tok);
    } else {
        throw BadJson("unexpected token");
    }
}

template <typename T>
std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>, T> read_actual_scalar(token tok) {
    check_token<int_token>(tok);
    return extract<T, int_token>(tok);
}

template <typename T>
std::enable_if_t<std::is_same_v<std::string, T>, T> read_actual_scalar(token tok) {
    check_token<string_token>(tok);
    return std::move(std::get<string_token>(tok).value);
}

template <typename T>
T read_scalar(std::istream& stream) {
    auto tok = next_token(stream);
    return read_actual_scalar<T>(std::move(tok));
}

template <typename T>
std::optional<T> read_optional_scalar(std::istream& stream) {
    auto tok = next_token(stream);
    if(match_token<null_token>(tok)) {
        return std::optional<T>{};
    } else {
        return read_actual_scalar<T>(tok);
    }
}

template <typename T, typename F>
void read_optional_mapping(std::istream& stream, std::optional<T>& target, F&& consumer) {
    auto tok = next_token(stream);
    if(match_token<null_token>(tok)) {
        target.reset();
    } else {
        check_token<start_mapping_token>(tok);

        if(!target) {
            target = T{};
        }

        read_members(stream, *target, std::forward<F>(consumer));
    }
}

template <typename T, typename F>
void read_optional_sequence(std::istream& stream, std::optional<T>& target, F&& consumer) {
    auto tok = next_token(stream);
    if(match_token<null_token>(tok)) {
        target.reset();
    } else {
        check_token<start_sequence_token>(tok);

        if(!target) {
            target = T{};
        }

        read_elements(stream, *target, std::forward<F>(consumer));
    }
}

} // namespace minijson

namespace {

using namespace minijson;

struct tokenizer_testcase {
    std::string        input;
    std::vector<token> tokens;
};

class tokenizer_test : public testing::TestWithParam<tokenizer_testcase> {
};

std::string token_s(const token& t) {
    return std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, string_token>) {
            return arg.value;
        } else if constexpr(std::is_same_v<T, int_token>) {
            return arg.value;
        } else if constexpr(std::is_same_v<T, float_token>) {
            return arg.value;
        } else {
            return std::to_string(t.index());
        }
    },
                      t);
}

TEST_P(tokenizer_test, tokens) {
    auto testcase = GetParam();

    std::istringstream stream(testcase.input);

    for(auto&& expected : testcase.tokens) {
        token actual = next_token(stream);
        EXPECT_EQ(token_s(expected), token_s(actual));
    }
}

tokenizer_testcase tokenizer_testcases[] =
    {
        // empty
        {"", {eof_token{}}},

        // simple chars
        {"{", {start_mapping_token{}}},
        {"}", {end_mapping_token{}}},
        {"[", {start_sequence_token{}}},
        {"]", {end_sequence_token{}}},
        {",", {separator_token{}}},
        {":", {mapper_token{}}},

        // literals
        {"true", {true_token{}}},
        {"false", {false_token{}}},
        {"null", {null_token{}}},

        // numbers
        {"1", {int_token{"1"}}},
        {"12 ", {int_token{"12"}}},
        {"3.14", {float_token{"3.14"}}},
        {"2e7", {float_token{"2e7"}}},
        {"23e+2", {float_token{"23e+2"}}},
        {"23E-2", {float_token{"23E-2"}}},
        {"-2", {int_token{"-2"}}},
        {"+2.71", {float_token{"+2.71"}}},

        // strings
        {"\"Klaas de Vries\"", {string_token{"Klaas de Vries"}}},
        {"\"with \\\"quotes\\\"", {string_token{"with \"quotes\""}}},
        {"\"\\\\\"", {string_token{"\\"}}},
        {"\"/\"", {string_token{"/"}}},
        {"\"\\b\"", {string_token{"\b"}}},
        {"\"\\f\"", {string_token{"\f"}}},
        {"\"\\n\"", {string_token{"\n"}}},
        {"\"\\r\"", {string_token{"\r"}}},
        {"\"\\t\"", {string_token{"\t"}}},
        {"\"\\ud582\"", {string_token{"\xd5\x82"}}},
        {"\"noot\"", {string_token{"noot"}}},

        // skip whitespace
        {" \t", {eof_token{}}},
        {"\t{ ", {start_mapping_token{}}},
        {"\n[", {start_sequence_token{}}},

        // serial
        {"3 ,{\t}\"blah\" 2.72", {int_token{"3"}, separator_token{}, start_mapping_token{}, end_mapping_token{}, string_token{"blah"}, float_token{"2.72"}, eof_token{}}},
        {"{\"aap\": \"noot\"}\n", {start_mapping_token{}, string_token{"aap"}, mapper_token{}, string_token{"noot"}, end_mapping_token{}, eof_token{}}},
};

INSTANTIATE_TEST_SUITE_P(test_tokenizer,
                         tokenizer_test,
                         testing::ValuesIn(tokenizer_testcases));

TEST(tokenizer, invalid_utf8) {
    std::stringstream stream("\"\\ug582\"");
    EXPECT_THROW(next_token(stream), BadJson);
}

TEST(tokenizer, bad_literal) {
    std::stringstream stream("trfalse");
    EXPECT_THROW(next_token(stream), BadJson);
}

TEST(tokenizer, bad_token) {
    std::stringstream stream("!");
    EXPECT_THROW(next_token(stream), BadJson);
}

struct Nested {
    std::string value;
};

struct Struct {
    double                          x{0.0};
    double                          y{0.0};
    int                             n{0};
    bool                            b1{false};
    bool                            b2{true};
    std::string                     s;
    std::vector<int>                v;
    std::optional<int>              o1;
    std::optional<int>              o2;
    Nested                          nested;
    std::optional<Nested>           o3;
    std::optional<Nested>           o4;
    std::optional<std::vector<int>> o5;
    std::optional<std::vector<int>> o6;
};

TEST(parser, read_mapping) {
    std::stringstream stream(R"(
{
    "x": 3.14,
    "y": 2.72,
    "n": 123,
    "b1": true,
    "b2": false,
    "s": "abc",
    "v": [1, 2, 3],
    "o1": null,
    "o2": 456,
    "nested": {
        "value": "def"
    },
    "o3": null,
    "o4": {
        "value": "abc"
    },
    "o5": null,
    "o6": [4, 5, 6]
})");
    Struct            p;
    read_mapping(stream, p, [](std::istream& stream, Struct& target, std::string_view key) {
        if(key == "x") {
            target.x = read_scalar<double>(stream);
        } else if(key == "y") {
            target.y = read_scalar<double>(stream);
        } else if(key == "n") {
            target.n = read_scalar<int>(stream);
        } else if(key == "b1") {
            target.b1 = read_scalar<bool>(stream);
        } else if(key == "b2") {
            target.b2 = read_scalar<bool>(stream);
        } else if(key == "s") {
            target.s = read_scalar<std::string>(stream);
        } else if(key == "v") {
            read_sequence(stream, target.v, [&](std::istream& stream, std::vector<int>& v) {
                v.push_back(read_scalar<int>(stream));
            });
        } else if(key == "o1") {
            target.o1 = read_optional_scalar<int>(stream);
        } else if(key == "o2") {
            target.o2 = read_optional_scalar<int>(stream);
        } else if(key == "nested") {
            read_mapping(stream, target.nested, [](std::istream& s, Nested& n, std::string_view k) {
                if(k == "value") {
                    n.value = read_scalar<std::string>(s);
                }
            });
        } else if(key == "o3") {
            read_optional_mapping(stream, target.o3, [](std::istream& s, Nested& n, std::string_view k) {
                if(k == "value") {
                    n.value = read_scalar<std::string>(s);
                }
            });
        } else if(key == "o4") {
            read_optional_mapping(stream, target.o4, [](std::istream& s, Nested& n, std::string_view k) {
                if(k == "value") {
                    n.value = read_scalar<std::string>(s);
                }
            });
        } else if(key == "o5") {
            read_optional_sequence(stream, target.o5, [](std::istream& s, std::vector<int>& v) {
                v.push_back(read_scalar<int>(s));
            });
        } else if(key == "o6") {
            read_optional_sequence(stream, target.o6, [](std::istream& s, std::vector<int>& v) {
                v.push_back(read_scalar<int>(s));
            });
        }
    });
    match_and_consume<eof_token>(stream);

    EXPECT_EQ(3.14, p.x);
    EXPECT_EQ(2.72, p.y);
    EXPECT_EQ(123, p.n);
    EXPECT_TRUE(p.b1);
    EXPECT_FALSE(p.b2);
    EXPECT_EQ("abc", p.s);

    std::vector<int> ve{1, 2, 3};
    EXPECT_EQ(ve, p.v);

    EXPECT_EQ(std::optional<int>(), p.o1);
    EXPECT_EQ(std::optional<int>(456), p.o2);

    EXPECT_EQ("def", p.nested.value);
    EXPECT_FALSE(p.o3);
    EXPECT_EQ("abc", p.o4->value);

    EXPECT_FALSE(p.o5);
    ve = {4, 5, 6};
    EXPECT_EQ(ve, *p.o6);
}

} // namespace
} // namespace valuetypes
