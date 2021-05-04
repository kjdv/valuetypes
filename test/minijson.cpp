#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

namespace valuetypes {

using namespace std;

// TBD: how to include minijson in generated code? As separate library (complicating integrations into target source), or
//      part of generated code (duplication)
//      For now: dirty and practical: develop here and copy-paste into generated source

// this does not need to be in the minijson namespace:
template <typename T>
struct is_optional : false_type {};

template <typename T>
struct is_optional<optional<T>> : true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

namespace minijson { // declarations

class BadJson : public runtime_error {
  public:
    using runtime_error::runtime_error;
};

// tokens
struct start_mapping_token {};
struct end_mapping_token {};
struct start_sequence_token {};
struct end_sequence_token {};
struct separator_token {};
struct mapper_token {};
struct string_token {
    string value;
};
struct int_token {
    string value;
};
struct float_token {
    string value;
};
struct true_token {};
struct false_token {};
struct null_token {};
struct eof_token {};

using token = variant<
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

class Tokenizer {
  public:
    Tokenizer(istream* stream)
      : d_stream(stream) {
        assert(d_stream);
        advance();
    }

    const token& advance();

    const token& current() const noexcept {
        return d_current;
    }

  private:
    istream* d_stream;
    token    d_current;
};

class Parser {
  public:
    Parser(istream* stream)
      : d_tok(stream) {}

    template <typename T, typename F> // signature of F should be void(Parser &p, T& target, string_view key)
    enable_if_t<!is_optional_v<T>, T> read_mapping(F&& consume);
    template <typename T, typename F> // signature of F should be void(Parser &p, T& target, string_view key)
    enable_if_t<is_optional_v<T>, T> read_mapping(F&& consume);

    template <typename T, typename F> // signature of F should be void(Parser &p, T& target)
    enable_if_t<!is_optional_v<T>, T> read_sequence(F&& consume);
    template <typename T, typename F> // signature of F should be void(Parser &p, T& target)
    enable_if_t<is_optional_v<T>, T> read_sequence(F&& consume);

    template <typename T>
    enable_if_t<!is_optional_v<T>, T> read_scalar();
    template <typename T>
    enable_if_t<is_optional_v<T>, T> read_scalar();

    bool is_eof() const noexcept;

  private:
    template <typename T, typename F>
    void read_members(T& target, F&& consume);

    template <typename T, typename F>
    void read_kvpair(T& target, F&& consume);

    template <typename T, typename F>
    void read_elements(T& target, F&& consume);

    Tokenizer d_tok;
};

} // namespace minijson

namespace minijson { // definitions

template <typename T>
constexpr bool match_token(const token& t) noexcept {
    return holds_alternative<T>(t);
}

template <typename T>
void check_token(const token& t) {
    if(!match_token<T>(t)) {
        throw BadJson("unexpected token");
    }
}

template <typename T, typename F>
enable_if_t<!is_optional_v<T>, T> Parser::read_mapping(F&& consume) {
    check_token<start_mapping_token>(d_tok.current());
    d_tok.advance();

    T target{};
    read_members(target, forward<F>(consume));

    check_token<end_mapping_token>(d_tok.current());
    d_tok.advance();

    return target;
}

template <typename T, typename F>
enable_if_t<is_optional_v<T>, T> Parser::read_mapping(F&& consume) {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        return T{};
    } else {
        return read_mapping<typename T::value_type>(forward<F>(consume));
    }
}

template <typename T, typename F>
enable_if_t<!is_optional_v<T>, T> Parser::read_sequence(F&& consume) {
    check_token<start_sequence_token>(d_tok.current());
    d_tok.advance();

    T target{};
    read_elements(target, forward<F>(consume));

    check_token<end_sequence_token>(d_tok.current());
    d_tok.advance();

    return target;
}

template <typename T, typename F>
enable_if_t<is_optional_v<T>, T> Parser::read_sequence(F&& consume) {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        return T{};
    } else {
        return read_sequence<typename T::value_type>(forward<F>(consume));
    }
}

bool Parser::is_eof() const noexcept {
    return match_token<eof_token>(d_tok.current());
}

template <typename T, typename F>
void Parser::read_members(T& target, F&& consume) {
    while(match_token<string_token>(d_tok.current())) {
        read_kvpair(target, forward<F>(consume));

        if(match_token<separator_token>(d_tok.current())) {
            d_tok.advance();
        } else {
            break;
        }
    }
}

template <typename T, typename F>
void Parser::read_kvpair(T& target, F&& consume) {
    check_token<string_token>(d_tok.current());
    string key = get<string_token>(d_tok.current()).value;

    check_token<mapper_token>(d_tok.advance());
    d_tok.advance();

    consume(*this, target, key);
}

template <typename T, typename F>
void Parser::read_elements(T& target, F&& consume) {
    while(!match_token<end_sequence_token>(d_tok.current())) {
        consume(*this, target);

        if(match_token<separator_token>(d_tok.current())) {
            d_tok.advance();
        } else {
            break;
        }
    }
}

template <typename T, typename Tok>
T extract(const token& tok) {
    istringstream istream(get<Tok>(tok).value);
    T             target;
    istream >> target;
    return target;
}

template <typename T>
enable_if_t<is_same_v<bool, T>, T> read_actual_scalar(const token& tok) {
    if(match_token<true_token>(tok)) {
        return true;
    } else if(match_token<false_token>(tok)) {
        return false;
    } else {
        throw BadJson("expected either 'true' or 'false");
    }
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> read_actual_scalar(const token& tok) {
    if(match_token<float_token>(tok)) {
        return extract<T, float_token>(tok);
    } else if(match_token<int_token>(tok)) {
        return extract<T, int_token>(tok);
    } else {
        throw BadJson("unexpected token");
    }
}

template <typename T>
enable_if_t<is_integral_v<T> && !is_same_v<bool, T>, T> read_actual_scalar(const token& tok) {
    check_token<int_token>(tok);
    return extract<T, int_token>(tok);
}

template <typename T>
enable_if_t<is_same_v<string, T>, T> read_actual_scalar(const token& tok) {
    check_token<string_token>(tok);
    return get<string_token>(tok).value;
}

template <typename T>
enable_if_t<!is_optional_v<T>, T> Parser::read_scalar() {
    T v = read_actual_scalar<T>(d_tok.current());
    d_tok.advance();
    return v;
}

template <typename T>
enable_if_t<is_optional_v<T>, T> Parser::read_scalar() {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        return T{};
    } else {
        return read_scalar<typename T::value_type>();
    }
}

const int eof = char_traits<char>::eof();

int non_ws(istream& str) noexcept {
    char c = 0;
    while(str.get(c)) {
        if(!isspace(c))
            return c;
    }
    return eof;
}

void extract_literal(istream& input, string_view tail) {
    for(char e : tail) {
        int c = input.get();
        if(c != e)
            throw BadJson("unexpected char in literal");
    }
}

token extract_number(istream& input, char head) noexcept {
    bool is_float  = false;
    bool had_point = false;
    bool had_exp   = false;

    string value;
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

string extract_utf8(istream& input) {
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

    string value;

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

string_token extract_string(istream& input) {
    string value;

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

const token& Tokenizer::advance() {
    int c = non_ws(*d_stream);
    if(c != eof) {
        switch(c) {
        case '{':
            d_current = start_mapping_token{};
            break;
        case '}':
            d_current = end_mapping_token{};
            break;
        case '[':
            d_current = start_sequence_token{};
            break;
        case ']':
            d_current = end_sequence_token{};
            break;
        case ',':
            d_current = separator_token{};
            break;
        case ':':
            d_current = mapper_token{};
            break;

        case 't':
            extract_literal(*d_stream, "rue");
            d_current = true_token{};
            break;
        case 'f':
            extract_literal(*d_stream, "alse");
            d_current = false_token{};
            break;
        case 'n':
            extract_literal(*d_stream, "ull");
            d_current = null_token{};
            break;

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
            d_current = extract_number(*d_stream, c);
            break;

        case '"':
            d_current = extract_string(*d_stream);
            break;

        default:
            throw BadJson("unexpected token");
        }
    } else {
        d_current = eof_token{};
    }
    return current();
}

} // namespace minijson

namespace {

using namespace minijson;

struct tokenizer_testcase {
    string        input;
    vector<token> tokens;
};

class tokenizer_test : public testing::TestWithParam<tokenizer_testcase> {
};

string token_s(const token& t) {
    return visit([&](auto&& arg) {
        using T = decay_t<decltype(arg)>;
        if constexpr(is_same_v<T, string_token>) {
            return arg.value;
        } else if constexpr(is_same_v<T, int_token>) {
            return arg.value;
        } else if constexpr(is_same_v<T, float_token>) {
            return arg.value;
        } else {
            return to_string(t.index());
        }
    },
                 t);
}

TEST_P(tokenizer_test, tokens) {
    auto testcase = GetParam();

    istringstream stream(testcase.input);
    Tokenizer     tok(&stream);

    for(auto&& expected : testcase.tokens) {
        token actual = tok.current();
        tok.advance();
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
    stringstream stream("\"\\ug582\"");
    try {
        Tokenizer tok(&stream);
        FAIL();
    } catch(const BadJson&) {
        SUCCEED();
    }
}

TEST(tokenizer, bad_literal) {
    stringstream stream("trfalse");
    try {
        Tokenizer tok(&stream);
        FAIL();
    } catch(const BadJson&) {
        SUCCEED();
    }
}

TEST(tokenizer, bad_token) {
    stringstream stream("!");
    try {
        Tokenizer tok(&stream);
        FAIL();
    } catch(const BadJson&) {
        SUCCEED();
    }
}

struct Nested {
    string value;
};

struct Struct {
    double                x{0.0};
    double                y{0.0};
    int                   n{0};
    bool                  b1{false};
    bool                  b2{true};
    string                s;
    vector<int>           v;
    optional<int>         o1;
    optional<int>         o2;
    Nested                nested;
    optional<Nested>      o3;
    optional<Nested>      o4;
    optional<vector<int>> o5;
    optional<vector<int>> o6;
};

TEST(parser, read_mapping) {
    stringstream stream(R"(
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
    Parser       parser(&stream);
    Struct       actual = parser.read_mapping<Struct>([](Parser& p, Struct& target, string_view key) {
        if(key == "x") {
            target.x = p.read_scalar<double>();
        } else if(key == "y") {
            target.y = p.read_scalar<double>();
        } else if(key == "n") {
            target.n = p.read_scalar<int>();
        } else if(key == "b1") {
            target.b1 = p.read_scalar<bool>();
        } else if(key == "b2") {
            target.b2 = p.read_scalar<bool>();
        } else if(key == "s") {
            target.s = p.read_scalar<string>();
        } else if(key == "v") {
            target.v = p.read_sequence<vector<int>>([](Parser& p2, vector<int>& v) {
                v.push_back(p2.read_scalar<int>());
            });
        } else if(key == "o1") {
            target.o1 = p.read_scalar<optional<int>>();
        } else if(key == "o2") {
            target.o2 = p.read_scalar<optional<int>>();
        } else if(key == "nested") {
            target.nested = p.read_mapping<Nested>([](Parser& p2, Nested& n, string_view k) {
                if(k == "value") {
                    n.value = p2.read_scalar<string>();
                }
            });
        } else if(key == "o3") {
            target.o3 = p.read_mapping<optional<Nested>>([](Parser& p2, Nested& n, string_view k) {
                if(k == "value") {
                    n.value = p2.read_scalar<string>();
                }
            });
        } else if(key == "o4") {
            target.o4 = p.read_mapping<optional<Nested>>([](Parser& p2, Nested& n, string_view k) {
                if(k == "value") {
                    n.value = p2.read_scalar<string>();
                }
            });
        } else if(key == "o5") {
            target.o5 = p.read_sequence<optional<vector<int>>>([](Parser& p2, vector<int>& v) {
                v.push_back(p2.read_scalar<int>());
            });
        } else if(key == "o6") {
            target.o6 = p.read_sequence<optional<vector<int>>>([](Parser& p2, vector<int>& v) {
                v.push_back(p2.read_scalar<int>());
            });
        }
    });

    EXPECT_TRUE(parser.is_eof());

    EXPECT_EQ(3.14, actual.x);
    EXPECT_EQ(2.72, actual.y);
    EXPECT_EQ(123, actual.n);
    EXPECT_TRUE(actual.b1);
    EXPECT_FALSE(actual.b2);
    EXPECT_EQ("abc", actual.s);

    vector<int> ve{1, 2, 3};
    EXPECT_EQ(ve, actual.v);

    EXPECT_EQ(optional<int>(), actual.o1);
    EXPECT_EQ(optional<int>(456), actual.o2);

    EXPECT_EQ("def", actual.nested.value);
    EXPECT_FALSE(actual.o3);
    EXPECT_EQ("abc", actual.o4->value);

    EXPECT_FALSE(actual.o5);
    ve = {4, 5, 6};
    EXPECT_EQ(ve, *actual.o6);
}

} // namespace
} // namespace valuetypes
