#include "valuetypes.h"
#include <algorithm>
#include <cassert>
#include <composite/make.hh>
#include <iomanip>
#include <kjson/builder.hh>
#include <kjson/json.hh>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace sp { 

bool operator==(const Nested &a, const Nested &b) noexcept {
    return
        std::tie(a.s) ==
        std::tie(b.s);
}

bool operator!=(const Nested &a, const Nested &b) noexcept {
    return !(a == b);
}

bool operator==(const Compound &a, const Compound &b) noexcept {
    return
        std::tie(a.a, a.b) ==
        std::tie(b.a, b.b);
}

bool operator!=(const Compound &a, const Compound &b) noexcept {
    return !(a == b);
}

bool operator==(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return
        std::tie(a.v) ==
        std::tie(b.v);
}

bool operator!=(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return !(a == b);
}

bool operator==(const VectorTo &a, const VectorTo &b) noexcept {
    return
        std::tie(a.v) ==
        std::tie(b.v);
}

bool operator!=(const VectorTo &a, const VectorTo &b) noexcept {
    return !(a == b);
}


bool operator<(const Nested &a, const Nested &b) noexcept {
    return
        std::tie(a.s) <
        std::tie(b.s);
}

bool operator<=(const Nested &a, const Nested &b) noexcept {
    return !(b < a);
}

bool operator>(const Nested &a, const Nested &b) noexcept {
    return b < a;
}

bool operator>=(const Nested &a, const Nested &b) noexcept {
    return !(a < b);
}

bool operator<(const Compound &a, const Compound &b) noexcept {
    return
        std::tie(a.a, a.b) <
        std::tie(b.a, b.b);
}

bool operator<=(const Compound &a, const Compound &b) noexcept {
    return !(b < a);
}

bool operator>(const Compound &a, const Compound &b) noexcept {
    return b < a;
}

bool operator>=(const Compound &a, const Compound &b) noexcept {
    return !(a < b);
}

bool operator<(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return
        std::tie(a.v) <
        std::tie(b.v);
}

bool operator<=(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return !(b < a);
}

bool operator>(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return b < a;
}

bool operator>=(const OptionalVectors &a, const OptionalVectors &b) noexcept {
    return !(a < b);
}

bool operator<(const VectorTo &a, const VectorTo &b) noexcept {
    return
        std::tie(a.v) <
        std::tie(b.v);
}

bool operator<=(const VectorTo &a, const VectorTo &b) noexcept {
    return !(b < a);
}

bool operator>(const VectorTo &a, const VectorTo &b) noexcept {
    return b < a;
}

bool operator>=(const VectorTo &a, const VectorTo &b) noexcept {
    return !(a < b);
}



} // } // namespace sp

// start iostream_definitions.cpp.inja

namespace sp { 
namespace {

template <typename T>
struct is_optional : std::false_type
{};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type
{};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

template <typename T>
struct is_vector : std::false_type
{};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type
{};

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

namespace json {

/*
 * Grammar:
 *
 * json
 *   element
 *
 * value
 *   object | array | string | number | "true" | "false" |  "null"
 *
 * object
 *   '{' ws '}' | '{' members '}'
 *
 * members
 *   member | member ',' members
 *
 * member
 *   ws string ws ':' element
 *
 * array
 *   '[' ws ']' | '[' elements ']'
 *
 * elements
 *   element | element ',' elements
 *
 * element
 *   ws value ws
 *
 */

class json_error : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

constexpr const int eof = std::char_traits<char>::eof();

struct token {
    enum class type_t {
        e_start_mapping,  // {
        e_end_mapping,    // }
        e_start_sequence, // [
        e_end_sequence,   // ]
        e_separator,      // ,
        e_mapper,         // :
        e_string,
        e_int,
        e_uint,
        e_float,
        e_true,  // true
        e_false, // false
        e_null,  // null
        e_eof,
    };

    type_t      tok{type_t::e_eof};
    std::string value{};
};

constexpr std::string to_string(token::type_t tok) {
    switch(tok) {
    case token::type_t::e_start_mapping:
        return "{";
    case token::type_t::e_end_mapping:
        return "}";
    case token::type_t::e_start_sequence:
        return "[";
    case token::type_t::e_end_sequence:
        return "]";
    case token::type_t::e_separator:
        return ",";
    case token::type_t::e_mapper:
        return ":";
    case token::type_t::e_string:
        return "quoted string";
    case token::type_t::e_int:
        return "integer";
    case token::type_t::e_uint:
        return "unsigned integer";
    case token::type_t::e_float:
        return "floating point number";
    case token::type_t::e_true:
        return "true";
    case token::type_t::e_false:
        return "false";
    case token::type_t::e_null:
        return "null";
    case token::type_t::e_eof:
        return "eof";
    }
    return "";
}

std::string to_string(const token& tok) {
    return to_string(tok.tok) + (tok.value.empty() ? "" : std::string(" '") + tok.value + "'");
}

int non_ws(std::istream& str) {
    char c = 0;
    while(str.get(c)) {
        if(!isspace(c))
            return c;
    }
    return eof;
}

void extract_literal(std::istream& input, char head, std::string const& tail) {
    for(char e : tail) {
        int c = input.get();
        if(c != e) {
            throw json_error(std::string("expected literal ") + std::string(1, head) + tail + ", found char " + std::string(1, c));
        }
    }
}

token extract_number(std::istream& input, char head) {
    bool is_float    = false;
    bool had_point   = false;
    bool had_exp     = false;
    bool is_negative = head == '-';

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

    return token{is_float ? token::type_t::e_float : (is_negative ? token::type_t::e_int : token::type_t::e_uint), std::move(value)};
}

constexpr bool is_hex(char c) noexcept {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

constexpr uint8_t single_decode(char h) noexcept {
    if(h >= '0' && h <= '9')
        return h - '0';
    else if(h >= 'a' && h <= 'f')
        return h - 'a' + 10;
    else if(h >= 'A' && h <= 'F')
        return h - 'A' + 10;

    assert(false && "unreachable: invalid hex char " && h);
    return 0;
}

std::string extract_utf8(std::istream& input) {
    char32_t wc = 0;
    for(size_t i = 0; i < 4; ++i) {
        int c = input.get();
        if(c == eof)
            break;

        if(!is_hex(c))
            throw json_error(std::string("expected hex digit, found ") + std::string(1, c));

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

token extract_string(std::istream& input) {
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

            case 'u': {
                value += extract_utf8(input);
            } break;

            default:
                value += c;
                break;
            }
        } else
            value += c;
    }

    return token{token::type_t::e_string, std::move(value)};
}

token next_token(std::istream& input) {
    int c = non_ws(input);
    if(c != eof) {
        switch(c) {
        case '{':
            return token{token::type_t::e_start_mapping};
        case '}':
            return token{token::type_t::e_end_mapping};
        case '[':
            return token{token::type_t::e_start_sequence};
        case ']':
            return token{token::type_t::e_end_sequence};
        case ',':
            return token{token::type_t::e_separator};
        case ':':
            return token{token::type_t::e_mapper};

        case 't':
            extract_literal(input, 't', "rue");
            return token{token::type_t::e_true, "true"};
        case 'f':
            extract_literal(input, 'f', "alse");
            return token{token::type_t::e_false, "false"};
        case 'n':
            extract_literal(input, 'n', "ull");
            return token{token::type_t::e_null, "null"};

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
            throw json_error(std::string("unexpected token ") + std::string(1, c));
        }
    }
    return token{token::type_t::e_eof};
}

class tokenizer {
  public:
    tokenizer(std::istream& input)
      : d_input(input) {}

    token next() {
        if(d_next) {
            token t = std::move(d_next.value());
            d_next.reset();
            return t;
        }
        return next_token(d_input);
    }

    const token& peek() {
        if(!d_next) {
            d_next = next_token(d_input);
        }
        return d_next.value();
    }

  private:
    std::istream&        d_input;
    std::optional<token> d_next;
};

void expect(token::type_t e, const token& actual) {
    if(actual.tok != e) {
        throw json_error(std::string("expected ") + to_string(e) + ", found " + to_string(actual));
    }
}

token expect_and_consume(tokenizer& input, token::type_t e) {
    auto t = input.next();
    expect(e, t);
    return t;
}

template <typename T>
void value(tokenizer& input, T& target);

template <typename T>
void object(tokenizer& input, T& target);

template <typename T>
void members(tokenizer& input, T& target);

template <typename T>
void member(tokenizer& input, T& target);

template <typename T>
void array(tokenizer& input, T& target);

template <typename T>
void element(tokenizer& input, T& target);

template <typename T>
void elements(tokenizer& input, T& target);

template <typename T>
void value(tokenizer& input, T& target) {
    if constexpr(is_optional_v<T>) {
        if(input.peek().tok == token::type_t::e_null) {
            target.reset();
            input.next();
        } else {
            target.emplace();
            value(input, *target);
        }
    } else if constexpr(is_vector_v<T>) {
        array(input, target);
    } else if constexpr(std::is_same_v<std::string, T>) {
        auto tok = input.next();
        expect(token::type_t::e_string, tok);
        target = std::move(tok.value);
    } else if constexpr(std::is_same_v<bool, T>) {
        auto tok = input.next();
        switch(tok.tok) {
        case token::type_t::e_true:
            target = true;
            break;
        case token::type_t::e_false:
            target = false;
            break;
        default:
            throw json_error("expected 'true' or 'false', found: " + to_string(tok));
        }
    } else if constexpr(std::is_integral_v<T> && std::is_unsigned_v<T>) {
        auto tok = input.next();
        expect(token::type_t::e_uint, tok);
        char* c;
        target = std::strtoull(tok.value.c_str(), &c, 10);
    } else if constexpr(std::is_integral_v<T>) {
        auto tok = input.next();
        switch(tok.tok) {
        case token::type_t::e_uint: {
            char* c;
            target = std::strtoull(tok.value.c_str(), &c, 10);
            break;
        }
        case token::type_t::e_int: {
            char* c;
            target = std::strtoll(tok.value.c_str(), &c, 10);
            break;
        }
        default:
            throw json_error("expected integer, found: " + to_string(tok));
        }
    } else if constexpr(std::is_floating_point_v<T>) {
        auto tok = input.next();
        switch(tok.tok) {
        case token::type_t::e_uint: {
            char* c;
            target = std::strtoull(tok.value.c_str(), &c, 10);
            break;
        }
        case token::type_t::e_int: {
            char* c;
            target = std::strtoll(tok.value.c_str(), &c, 10);
            break;
        }
        case token::type_t::e_float: {
            char* c;
            target = std::strtod(tok.value.c_str(), &c);
            break;
        }
        default:
            throw json_error("expected number, found: " + to_string(tok));
        }
    } else if constexpr(std::is_class_v<T>) {
        object(input, target);
    } else {
        assert(false && "type deduction failed");
    }
}

// forward declarations
void member(tokenizer& input, sp::Nested& target);
void member(tokenizer& input, sp::Compound& target);
void member(tokenizer& input, sp::OptionalVectors& target);
void member(tokenizer& input, sp::VectorTo& target);

template <typename T>
void object(tokenizer& input, T& target) {
    // object
    //   '{' ws '}' | '{' members '}'
    expect_and_consume(input, token::type_t::e_start_mapping);

    if(input.peek().tok == token::type_t::e_string) {
        members(input, target);
    }

    expect_and_consume(input, token::type_t::e_end_mapping);
}

template <typename T>
void members(tokenizer& input, T& target) {
    // members
    //   member | member ',' members
    while(true) {
        member(input, target);
        if(input.peek().tok != token::type_t::e_separator) {
            break;
        }
        input.next();
    }
}

token extract_key(tokenizer& input) {
    auto tok = expect_and_consume(input, token::type_t::e_string);
    expect_and_consume(input, token::type_t::e_mapper);
    return tok;
}

template <typename T>
void member(tokenizer& input, T& target) {
    // member
    //   ws string ws ':' element

    extract_key(input);
    element(input, target);
}

void member(tokenizer& input, sp::Nested& target) {
    auto key = extract_key(input);
    if(key.value == "s") {
        element(input, target.s);
    } else {
        // todo: tolerate this
        throw json_error(std::string("unknown key: ") + to_string(key));
    }
}

void member(tokenizer& input, sp::Compound& target) {
    auto key = extract_key(input);
    if(key.value == "a") {
        element(input, target.a);
    } else if(key.value == "b") {
        element(input, target.b);
    } else {
        // todo: tolerate this
        throw json_error(std::string("unknown key: ") + to_string(key));
    }
}

void member(tokenizer& input, sp::OptionalVectors& target) {
    auto key = extract_key(input);
    if(key.value == "v") {
        element(input, target.v);
    } else {
        // todo: tolerate this
        throw json_error(std::string("unknown key: ") + to_string(key));
    }
}

void member(tokenizer& input, sp::VectorTo& target) {
    auto key = extract_key(input);
    if(key.value == "v") {
        element(input, target.v);
    } else {
        // todo: tolerate this
        throw json_error(std::string("unknown key: ") + to_string(key));
    }
}

template <typename T>
void array(tokenizer& input, T& target) {
    // array
    //   '[' ws ']' | '[' elements ']'

    expect_and_consume(input, token::type_t::e_start_sequence);

    if(input.peek().tok != token::type_t::e_end_sequence) {
        elements(input, target);
    }

    expect_and_consume(input, token::type_t::e_end_sequence);
}

template <typename T>
void elements(tokenizer& input, T& target) {
    // elements
    //   element | element ',' elements
    static_assert(is_vector_v<T>, "expected a vector");

    target.clear();
    while(true) {
        target.emplace_back();
        element(input, target.back());

        if(input.peek().tok != token::type_t::e_separator) {
            break;
        }
        input.next();
    }
}

template <typename T>
void element(tokenizer& input, T& target) {
    // element
    //   ws value ws
    value(input, target);
}

} // namespace json

void to_kjson(kjson::builder& builder, const Nested& v);
void to_kjson(kjson::builder& builder, const Compound& v);
void to_kjson(kjson::builder &builder, const OptionalVectors &v);
void to_kjson(kjson::builder& builder, const VectorTo& v);

template <typename T>
void to_kjson(kjson::builder& builder, const T& v) {
    if constexpr (is_optional_v<T>) {
        if (!v) {
            builder.with_none();
        } else {
            to_kjson(builder, *v);
        }
    } else if constexpr (is_vector_v<T>) {
        builder.push_sequence();
        for (auto&& item : v) {
            to_kjson(builder, item);
        }
        builder.pop();
    } else {
        builder.value(v);
    }
}

void to_kjson(kjson::builder& builder, const Nested& v) {
    builder.push_mapping();
    builder.key("s");
    to_kjson(builder, v.s);
    builder.pop();
}

void to_kjson(kjson::builder &builder, const Compound &v) {
    builder.push_mapping();
    builder.key("a");
    to_kjson(builder, v.a);
    builder.key("b");
    to_kjson(builder, v.b);
    builder.pop();
}

void to_kjson(kjson::builder& builder, const OptionalVectors& v) {
    builder.push_mapping();
    builder.key("v");
    to_kjson(builder, v.v);
    builder.pop();
}
void to_kjson(kjson::builder &builder, const VectorTo &v) {
    builder.push_mapping();
    builder.key("v");
    to_kjson(builder, v.v);
    builder.pop();
}

} // anonymous namespace

void to_json(std::ostream& out, const Nested &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Nested &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const Compound &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Compound &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const OptionalVectors &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, OptionalVectors &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const VectorTo &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, VectorTo &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

} // namespace sp

namespace std {

std::ostream &operator<<(std::ostream& out, const sp::Nested &v) {
    sp::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, sp::Nested &v) {
    sp::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const sp::Compound &v) {
    sp::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, sp::Compound &v) {
    sp::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const sp::OptionalVectors &v) {
    sp::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, sp::OptionalVectors &v) {
    sp::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const sp::VectorTo &v) {
    sp::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, sp::VectorTo &v) {
    sp::from_json(in, v);
    return in;
}

} // namespace std

// end iostream_definitions.cpp.inja

// start hash_definitions.cpp.inja

namespace std {

namespace {

constexpr std::size_t combine(std::size_t a, std::size_t b) noexcept {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

template <typename T>
constexpr std::size_t base_hash(const T&v) noexcept {
    std::hash<T> hasher;
    return hasher(v);
}

template <typename T>
constexpr std::size_t base_hash(const std::vector<T> &v) noexcept {
    std::size_t h{0};
    std::hash<T> ih;
    for (auto&& item : v) {
        h = combine(h, ih(item));
    }
    return h;
}

template <typename T>
constexpr std::size_t base_hash(const std::optional<T> &v) noexcept {
    return v ? base_hash(*v) : 0;
}

std::size_t hash_combine() {
    return 0;
}

template <typename Head, typename... Tail>
std::size_t hash_combine(const Head &head, Tail... tail) {
    auto h = base_hash(head);
    auto t = hash_combine(std::forward<Tail>(tail)...);
    return combine(t, h);
}

} // anonymous namespace

std::size_t hash<sp::Nested>::operator()(const sp::Nested &v) const noexcept {
    return hash_combine(v.s);
}

std::size_t hash<sp::Compound>::operator()(const sp::Compound &v) const noexcept {
    return hash_combine(v.a, v.b);
}

std::size_t hash<sp::OptionalVectors>::operator()(const sp::OptionalVectors &v) const noexcept {
    return hash_combine(v.v);
}

std::size_t hash<sp::VectorTo>::operator()(const sp::VectorTo &v) const noexcept {
    return hash_combine(v.v);
}

} // namespace std

// end hash_definitions.cpp.inja
// start swap_definitions.cpp.inja

namespace std {

void swap(sp::Nested &a, sp::Nested &b) noexcept {
    swap(a.s, b.s);
}

void swap(sp::Compound &a, sp::Compound &b) noexcept {
    swap(a.a, b.a);
    swap(a.b, b.b);
}

void swap(sp::OptionalVectors &a, sp::OptionalVectors &b) noexcept {
    swap(a.v, b.v);
}

void swap(sp::VectorTo &a, sp::VectorTo &b) noexcept {
    swap(a.v, b.v);
}

} // namespace std

// end swap_definitions.cpp.inja
