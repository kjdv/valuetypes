#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
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

template <typename F>
void read_mapping(std::istream& stream, F&& consumer) {
    match_and_consume<start_mapping_token>(stream);

    auto tok = next_token(stream);
    while(match_token<string_token>(tok)) {
        auto& key = std::get<string_token>(tok).value;

        match_and_consume<mapper_token>(stream);

        consumer(stream, key);

        tok = next_token(stream);
        if(match_token<separator_token>(tok)) {
            tok = next_token(stream);
        } else {
            break;
        }
    }

    check_token<end_mapping_token>(tok);
}

template <typename T, typename Tok>
T extract(const token& tok) {
    std::istringstream istream(std::get<Tok>(tok).value);
    T                  target;
    istream >> target;
    return target;
}

template <typename T>
T read_scalar(std::istream& stream) {
    auto tok = next_token(stream);

    if constexpr(std::is_same_v<T, bool>) {
        if(match_token<true_token>(tok)) {
            return true;
        } else if(match_token<false_token>(tok)) {
            return false;
        } else {
            throw BadJson("expected either 'true' or 'false");
        }
    } else if constexpr(std::is_floating_point_v<T>) {
        if(match_token<float_token>(tok)) {
            return extract<T, float_token>(tok);
        } else if(match_token<int_token>(tok)) {
            return extract<T, int_token>(tok);
        } else {
            throw BadJson("unexpected token");
        }
    } else if constexpr(std::is_integral_v<T>) {
        check_token<int_token>(tok);
        return extract<T, int_token>(tok);
    } else if constexpr(std::is_same_v<T, std::string>) {
        check_token<string_token>(tok);
        return std::get<string_token>(tok).value;
    } else {
        assert(false);
        return T{};
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

struct Struct {
    double      x{0.0};
    double      y{0.0};
    int         n{0};
    bool        b1{false};
    bool        b2{true};
    std::string s;
};

TEST(parser, read_mapping) {
    std::stringstream stream(R"({ "x": 3.14, "y": 2.72, "n": 123, "b1": true, "b2": false, "s": "abc"})");
    Struct            p;

    read_mapping(stream, [&](std::istream& stream, std::string_view key) {
        if(key == "x") {
            p.x = read_scalar<double>(stream);
        } else if(key == "y") {
            p.y = read_scalar<double>(stream);
        } else if(key == "n") {
            p.n = read_scalar<int>(stream);
        } else if(key == "b1") {
            p.b1 = read_scalar<bool>(stream);
        } else if(key == "b2") {
            p.b2 = read_scalar<bool>(stream);
        } else if(key == "s") {
            p.s = read_scalar<std::string>(stream);
        }
    });
    match_and_consume<eof_token>(stream);

    EXPECT_EQ(3.14, p.x);
    EXPECT_EQ(2.72, p.y);
    EXPECT_EQ(123, p.n);
    EXPECT_TRUE(p.b1);
    EXPECT_FALSE(p.b2);
    EXPECT_EQ("abc", p.s);
}

} // namespace
} // namespace valuetypes
