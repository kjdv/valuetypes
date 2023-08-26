#include "valuetypes.h"
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <algorithm>
#include <iomanip>
#include <limits>

namespace valuetypes { 

bool operator==(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return
        std::tie(a.type, a.optional, a.name) ==
        std::tie(b.type, b.optional, b.name);
}

bool operator!=(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return !(a == b);
}

bool operator==(const Member &a, const Member &b) noexcept {
    return
        std::tie(a.name, a.type, a.default_value, a.optional, a.value_type, a.value_types) ==
        std::tie(b.name, b.type, b.default_value, b.optional, b.value_type, b.value_types);
}

bool operator!=(const Member &a, const Member &b) noexcept {
    return !(a == b);
}

bool operator==(const Definition &a, const Definition &b) noexcept {
    return
        std::tie(a.name, a.members) ==
        std::tie(b.name, b.members);
}

bool operator!=(const Definition &a, const Definition &b) noexcept {
    return !(a == b);
}

bool operator==(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return
        std::tie(a.ns, a.types) ==
        std::tie(b.ns, b.types);
}

bool operator!=(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return !(a == b);
}


bool operator<(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return
        std::tie(a.type, a.optional, a.name) <
        std::tie(b.type, b.optional, b.name);
}

bool operator<=(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return !(b < a);
}

bool operator>(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return b < a;
}

bool operator>=(const TemplateParameter &a, const TemplateParameter &b) noexcept {
    return !(a < b);
}

bool operator<(const Member &a, const Member &b) noexcept {
    return
        std::tie(a.name, a.type, a.default_value, a.optional, a.value_type, a.value_types) <
        std::tie(b.name, b.type, b.default_value, b.optional, b.value_type, b.value_types);
}

bool operator<=(const Member &a, const Member &b) noexcept {
    return !(b < a);
}

bool operator>(const Member &a, const Member &b) noexcept {
    return b < a;
}

bool operator>=(const Member &a, const Member &b) noexcept {
    return !(a < b);
}

bool operator<(const Definition &a, const Definition &b) noexcept {
    return
        std::tie(a.name, a.members) <
        std::tie(b.name, b.members);
}

bool operator<=(const Definition &a, const Definition &b) noexcept {
    return !(b < a);
}

bool operator>(const Definition &a, const Definition &b) noexcept {
    return b < a;
}

bool operator>=(const Definition &a, const Definition &b) noexcept {
    return !(a < b);
}

bool operator<(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return
        std::tie(a.ns, a.types) <
        std::tie(b.ns, b.types);
}

bool operator<=(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return !(b < a);
}

bool operator>(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return b < a;
}

bool operator>=(const DefinitionStore &a, const DefinitionStore &b) noexcept {
    return !(a < b);
}



} // } // namespace valuetypes

// start iostream_definitions.cpp.inja

namespace valuetypes { 
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

struct sink {};

template <typename T>
void value(tokenizer& input, T& target);

template <typename T>
void value(tokenizer& input, sink target);

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
        // todo: in grammar this has to be a non-array, non-object value (but can be bool, int, string, float)
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
    } else {
        static_assert(std::is_class_v<T>, "type deduction failed");
        object(input, target);
    }
}

void value(tokenizer& input, sink target) {
    auto& tok = input.peek();
    switch(tok.tok) {
    case token::type_t::e_start_mapping:
        object(input, target);
        break;
    case token::type_t::e_start_sequence: {
        std::vector<sink> v;
        array(input, v);
        break;
    }
    case token::type_t::e_null:
    case token::type_t::e_false:
    case token::type_t::e_true:
    case token::type_t::e_uint:
    case token::type_t::e_int:
    case token::type_t::e_float:
    case token::type_t::e_string:
        // consume and ignore
        input.next();
        break;
    default:
        throw json_error(std::string("expected value, found " + to_string(tok)));
    }
}

// forward declarations
void member(tokenizer &input, TemplateParameter &target);
void member(tokenizer &input, Member &target);
void member(tokenizer &input, Definition &target);
void member(tokenizer &input, DefinitionStore &target);

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

void member(tokenizer &input, TemplateParameter &target) {
    auto key = extract_key(input);
    if(key.value == "type") {
        element(input, target.type);
    } 
    else if(key.value == "optional") {
        element(input, target.optional);
    } 
    else if(key.value == "name") {
        element(input, target.name);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(tokenizer &input, Member &target) {
    auto key = extract_key(input);
    if(key.value == "name") {
        element(input, target.name);
    } 
    else if(key.value == "type") {
        element(input, target.type);
    } 
    else if(key.value == "default_value") {
        element(input, target.default_value);
    } 
    else if(key.value == "optional") {
        element(input, target.optional);
    } 
    else if(key.value == "value_type") {
        element(input, target.value_type);
    } 
    else if(key.value == "value_types") {
        element(input, target.value_types);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(tokenizer &input, Definition &target) {
    auto key = extract_key(input);
    if(key.value == "name") {
        element(input, target.name);
    } 
    else if(key.value == "members") {
        element(input, target.members);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(tokenizer &input, DefinitionStore &target) {
    auto key = extract_key(input);
    if(key.value == "ns") {
        element(input, target.ns);
    } 
    else if(key.value == "types") {
        element(input, target.types);
    } 
    else {
        sink s;
        element(input, s);
    }
}

} // namespace json

template <typename T>
void to_json(std::ostream& out, const T& v) {
    if constexpr (is_optional_v<T>) {
        if (!v) {
            out << "null";
        } else {
            to_json(out, *v);
        }
    } else if constexpr (is_vector_v<T>) {
        out << "[ ";
        bool first{true};
        for (auto&& item : v) {
            if(first) {
                first = false;
            } else {
                out << ", ";
            }
            to_json(out, item);
        }
        out << ']';
    } else if constexpr(std::is_same_v<bool, T>) {
        out << std::boolalpha << v;
    } else if constexpr(std::is_floating_point_v<T>) {
        out.precision(std::numeric_limits<double>::max_digits10);
        out << v;
    } else if constexpr(std::is_same_v<std::string, T>) {
        out << std::quoted(v);
    } else {
        out << v;
    }
}

} // anonymous namespace

void to_json(std::ostream& out, const TemplateParameter &v) {
    out << "{ ";
    out << std::quoted("type") << ": ";
    to_json(out, v.type);
    out << ", ";
    out << std::quoted("optional") << ": ";
    to_json(out, v.optional);
    out << ", ";
    out << std::quoted("name") << ": ";
    to_json(out, v.name);
    out << '}';
}

void from_json(std::istream& in, TemplateParameter &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const Member &v) {
    out << "{ ";
    out << std::quoted("name") << ": ";
    to_json(out, v.name);
    out << ", ";
    out << std::quoted("type") << ": ";
    to_json(out, v.type);
    out << ", ";
    out << std::quoted("default_value") << ": ";
    to_json(out, v.default_value);
    out << ", ";
    out << std::quoted("optional") << ": ";
    to_json(out, v.optional);
    out << ", ";
    out << std::quoted("value_type") << ": ";
    to_json(out, v.value_type);
    out << ", ";
    out << std::quoted("value_types") << ": ";
    to_json(out, v.value_types);
    out << '}';
}

void from_json(std::istream& in, Member &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const Definition &v) {
    out << "{ ";
    out << std::quoted("name") << ": ";
    to_json(out, v.name);
    out << ", ";
    out << std::quoted("members") << ": ";
    to_json(out, v.members);
    out << '}';
}

void from_json(std::istream& in, Definition &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

void to_json(std::ostream& out, const DefinitionStore &v) {
    out << "{ ";
    out << std::quoted("ns") << ": ";
    to_json(out, v.ns);
    out << ", ";
    out << std::quoted("types") << ": ";
    to_json(out, v.types);
    out << '}';
}

void from_json(std::istream& in, DefinitionStore &v) {
    json::tokenizer input(in);
    json::value(input, v);
}

} // namespace valuetypes

namespace std {

std::ostream &operator<<(std::ostream& out, const valuetypes::TemplateParameter &v) {
    valuetypes::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, valuetypes::TemplateParameter &v) {
    valuetypes::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const valuetypes::Member &v) {
    valuetypes::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, valuetypes::Member &v) {
    valuetypes::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const valuetypes::Definition &v) {
    valuetypes::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, valuetypes::Definition &v) {
    valuetypes::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const valuetypes::DefinitionStore &v) {
    valuetypes::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, valuetypes::DefinitionStore &v) {
    valuetypes::from_json(in, v);
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

std::size_t hash<valuetypes::TemplateParameter>::operator()(const valuetypes::TemplateParameter &v) const noexcept {
    return hash_combine(v.type, v.optional, v.name);
}

std::size_t hash<valuetypes::Member>::operator()(const valuetypes::Member &v) const noexcept {
    return hash_combine(v.name, v.type, v.default_value, v.optional, v.value_type, v.value_types);
}

std::size_t hash<valuetypes::Definition>::operator()(const valuetypes::Definition &v) const noexcept {
    return hash_combine(v.name, v.members);
}

std::size_t hash<valuetypes::DefinitionStore>::operator()(const valuetypes::DefinitionStore &v) const noexcept {
    return hash_combine(v.ns, v.types);
}

} // namespace std

// end hash_definitions.cpp.inja
// start swap_definitions.cpp.inja

namespace std {

void swap(valuetypes::TemplateParameter &a, valuetypes::TemplateParameter &b) noexcept {
    swap(a.type, b.type);
    swap(a.optional, b.optional);
    swap(a.name, b.name);
}

void swap(valuetypes::Member &a, valuetypes::Member &b) noexcept {
    swap(a.name, b.name);
    swap(a.type, b.type);
    swap(a.default_value, b.default_value);
    swap(a.optional, b.optional);
    swap(a.value_type, b.value_type);
    swap(a.value_types, b.value_types);
}

void swap(valuetypes::Definition &a, valuetypes::Definition &b) noexcept {
    swap(a.name, b.name);
    swap(a.members, b.members);
}

void swap(valuetypes::DefinitionStore &a, valuetypes::DefinitionStore &b) noexcept {
    swap(a.ns, b.ns);
    swap(a.types, b.types);
}

} // namespace std

// end swap_definitions.cpp.inja
