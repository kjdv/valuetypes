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
#include <cassert>

{% if namespace %}namespace {{ namespace }} { {% endif %}

namespace {

using namespace std;

template <typename T>
struct is_optional: false_type
{};

template <typename T>
struct is_optional<optional<T>>: true_type
{};

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

template <typename T>
void to_json(ostream &out, const T &v) {
    using U = std::decay_t<T>;
    if constexpr (is_same_v<U, bool>) {
        out << boolalpha << v;
    } else if constexpr (is_floating_point_v<U>) {
        out << setprecision(18) << v;
    } else if constexpr (is_integral_v<U>) {
        out << v;
    } else if constexpr (is_same_v<U, string>) {
        out << quoted(v);
    } else {
        assert(false && "implement me");
    }
}

template <typename T>
T from_json(minijson::Parser &p) {
    return p.read_scalar<T>();
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

ostream &operator<<(ostream& out, const {{typedef.name }} &v) {
    to_json(out, v);
    return out;
}

istream &operator>>(istream& in, {{ typedef.name }} &v) {
    from_json(in, v);
    return in;
}

void to_json(std::ostream& out, const {{typedef.name }} &v) {
    out << "{ ";
## for member in typedef.members
    out << quoted("{{ member.name }}") << ": ";
    to_json(out, v.{{member.name}});
{% if not loop.is_last %}    out << ", ";{% endif %}
## endfor
    out << " }";
}

void from_json(istream& in, {{ typedef.name }} &v) {
    minijson::Parser parser(&in);
    v = parser.read_mapping<{{ typedef.name }}>([](minijson::Parser &p, {{ typedef.name }}& target, string_view key) {
## for member in typedef.members
        {% if not loop.is_first %}else {% endif %}if (key == "{{ member.name }}") {
            target.{{member.name}} = from_json<{{member.type}}>(p);
        }
## endfor
    });
}


## endfor

namespace {

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
    if (match_token<null_token>(d_tok.current())) {
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
    if (match_token<null_token>(d_tok.current())) {
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
} // anonymous namespace

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
