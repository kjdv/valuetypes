#include "templates.h"

const std::string valuetypes::source = R"raw(#include "{{ options.base_filename }}.h"
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

{% if namespace %}namespace {{ namespace }} { {% endif %}

namespace {

using namespace std;

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
std::optional<T> read_optional_scalar(std::istream &stream) {
    auto tok = next_token(stream);
    if (match_token<null_token>(tok)) {
        return std::optional<T>{};
    } else {
        return read_actual_scalar<T>(tok);
    }
}

template <typename T, typename F>
void read_optional_mapping(std::istream &stream, std::optional<T> &target, F&& consumer) {
    auto tok = next_token(stream);
    if (match_token<null_token>(tok)) {
        target.reset();
    } else {
        check_token<start_mapping_token>(tok);

        if (!target) {
            target = T{};
        }

        read_members(stream, *target, std::forward<F>(consumer));
    }
}

template <typename T, typename F>
void read_optional_sequence(std::istream &stream, std::optional<T> &target, F&& consumer) {
    auto tok = next_token(stream);
    if (match_token<null_token>(tok)) {
        target.reset();
    } else {
        check_token<start_sequence_token>(tok);

        if (!target) {
            target = T{};
        }

        read_elements(stream, *target, std::forward<F>(consumer));
    }
}

} // namespace minijson

template <typename T>
void to_json(ostream &out, T v) {
    out << setprecision(18) << v;
}

template <typename T>
void from_json(istream &in, T &v) {
    v = minijson::read_scalar<T>(in);
}

} // anonymous namespace

## for typedef in typedefs

bool operator==(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return tie(
## for member in typedef.members
        a.{{ member.name }}{% if not loop.is_last %},{% endif %}
## endfor
    ) == tie(
## for member in typedef.members
        b.{{ member.name }}{% if not loop.is_last %},{% endif %}
## endfor
    );
}

bool operator!=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return !(a == b);
}

bool operator<(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return tie(
## for member in typedef.members
        a.{{ member.name }}{% if not loop.is_last %},{% endif %}
## endfor
    ) < tie(
## for member in typedef.members
        b.{{ member.name }}{% if not loop.is_last %},{% endif %}
## endfor
    );
}

bool operator<=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return !(b < a);
}

bool operator>(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return b < a;
}

bool operator>=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept {
    return !(a < b);
}

ostream &operator<<(ostream& out, const {{typedef.name }}& v) {
    to_json(out, v);
    return out;
}

istream &operator>>(istream& in, {{ typedef.name }}& v) {
    from_json(in, v);
    return in;
}

void to_json(std::ostream& out, const {{typedef.name }}& v) {
    out << "{ ";
## for member in typedef.members
    out << quoted("{{ member.name }}") << ": ";
    to_json(out, v.{{member.name}});
{% if not loop.is_last %}    out << ", ";{% endif %}
## endfor
    out << " }";
}

void from_json(istream& in, {{ typedef.name }} &v) {
    minijson::read_mapping(in, v, [](istream &stream, {{ typedef.name }}& target, string_view key) {
## for member in typedef.members
    {% if not loop.is_first %}else {% endif %} if (key == "{{ member.name }}") {
        from_json(stream, target.{{member.name}});
    }
## endfor
    });
}


## endfor

{% if namespace %}} // namespace {{ namespace }}{% endif %}

namespace std {

namespace {

std::size_t hash_combine() {
    return 0;
}

template <typename Head, typename... Tail>
std::size_t hash_combine(const Head &head, Tail... tail) {
    hash<Head> hasher;
    auto h = hasher(head);
    auto t = hash_combine(std::forward<Tail>(tail)...);
    return t ^ (h + 0x9e3779b9 + (t << 6) + (t >> 2));
}

} // anonymous namespace

## for typedef in typedefs
std::size_t hash<{{typedef.namespace_name}}>::operator()(const {{typedef.namespace_name}} &v) const noexcept {
    return hash_combine(
## for member in typedef.members
        v.{{ member.name }}{% if not loop.is_last %},{% endif %}
## endfor
    );
}

## endfor
} // namespace std
)raw";
