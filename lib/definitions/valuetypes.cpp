#include "valuetypes.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace valuetypes { 

namespace {

template <typename T>
struct is_optional: std::false_type
{};

template <typename T>
struct is_optional<std::optional<T>>: std::true_type
{};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

template <typename T>
struct is_vector: std::false_type
{};

template <typename T>
struct is_vector<std::vector<T>>: std::true_type
{};

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

} // anonymous namespace

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

// start minijson_declarations.cpp.inja

namespace minijson { // declarations

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

std::string to_string(const token &t);

class Tokenizer {
  public:
    Tokenizer(std::istream* stream);

    const token& advance();

    const token& current() const noexcept {
        return d_current;
    }

  private:
    std::istream* d_stream;
    token    d_current;
};

class Parser {
  public:
    Parser(std::istream* stream)
      : d_tok(stream) {}

    template <typename T, typename F> // signature of F should be void(Parser &p, T& target, string_view key)
    void read_mapping(std::enable_if_t<!is_optional_v<T>, T> &target, F&& consume);
    template <typename T, typename F> // signature of F should be void(Parser &p, T& target, string_view key)
    void read_mapping(std::enable_if_t<is_optional_v<T>, T> &target, F&& consume);

    template <typename T, typename F> // signature of F should be void(Parser &p, T& target)
    void read_sequence(std::enable_if_t<!is_optional_v<T>, T> &target, F&& consume);
    template <typename T, typename F> // signature of F should be void(Parser &p, T& target)
    void read_sequence(std::enable_if_t<is_optional_v<T>, T> &target, F&& consume);

    template <typename T>
    void read_scalar(std::enable_if_t<!is_optional_v<T>, T> &target);
    template <typename T>
    void read_scalar(std::enable_if_t<is_optional_v<T>, T> &target);

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

std::string concat();

template <typename Head, typename... Tail>
std::string concat(const Head &head, Tail&&... tail);

} // namespace minijson

// end minijson_declarations.cpp.inja


template <typename T>
void to_json(std::ostream &out, const T &v) {
    using U = std::decay_t<T>;

    if constexpr (is_optional_v<U>) {
        if (!v) {
            out << "null";
        } else {
            to_json(out, *v);
        }
        return;
    }

    if constexpr (std::is_same_v<U, bool>) {
        out << std::boolalpha << v;
    } else if constexpr (std::is_floating_point_v<U>) {
        out << std::setprecision(18) << v;
    } else if constexpr (std::is_integral_v<U>) {
        out << v;
    } else if constexpr (std::is_same_v<U, std::string>) {
        out << std::quoted(v);
    } else if constexpr (is_vector_v<U>) {
        out << "[ ";
        bool first = true;
        for (auto&& item : v) {
            if (!first) {
                out << ", ";
            }
            to_json(out, item);
            first = false;
        }
        out << " ]";
    } else {
        assert(false && "implement me");
    }
}

void from_json(minijson::Parser &p, TemplateParameter &v);
void from_json(minijson::Parser &p, Member &v);
void from_json(minijson::Parser &p, Definition &v);
void from_json(minijson::Parser &p, DefinitionStore &v);

template <typename T>
void from_json(minijson::Parser &p, T &v) {
    p.read_scalar<T>(v);
}

template <typename T>
void from_json(minijson::Parser &p, std::vector<T> &v) {
    p.read_sequence<std::vector<T>>(v, [](minijson::Parser &p2, std::vector<T> &target) {
        target.emplace_back();
        from_json(p2, target.back());
    });
}

template <typename T>
void from_json(minijson::Parser &p, std::optional<std::vector<T>> &v) {
    p.read_sequence<std::optional<std::vector<T>>>(v, [](minijson::Parser &p2, std::vector<T> &target) {
        target.emplace_back();
        from_json(p2, target.back());
    });
}

void fill_struct(minijson::Parser &p, TemplateParameter& target, std::string_view key) {
    if (key == "type") {
        from_json(p, target.type);
    }
    else if (key == "optional") {
        from_json(p, target.optional);
    }
    else if (key == "name") {
        from_json(p, target.name);
    }
    else {
        throw minijson::BadJson(minijson::concat("unexpected key in struct of type TemplateParameter: ", std::string(key)));
    }
}

void from_json(minijson::Parser &p, std::optional<TemplateParameter> &v) {
    p.read_mapping<std::optional<TemplateParameter>>(v, [](minijson::Parser &p2, TemplateParameter& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void from_json(minijson::Parser &p, TemplateParameter &v) {
    p.read_mapping<TemplateParameter>(v, [](minijson::Parser &p2, TemplateParameter& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void fill_struct(minijson::Parser &p, Member& target, std::string_view key) {
    if (key == "name") {
        from_json(p, target.name);
    }
    else if (key == "type") {
        from_json(p, target.type);
    }
    else if (key == "default_value") {
        from_json(p, target.default_value);
    }
    else if (key == "optional") {
        from_json(p, target.optional);
    }
    else if (key == "value_type") {
        from_json(p, target.value_type);
    }
    else if (key == "value_types") {
        from_json(p, target.value_types);
    }
    else {
        throw minijson::BadJson(minijson::concat("unexpected key in struct of type Member: ", std::string(key)));
    }
}

void from_json(minijson::Parser &p, std::optional<Member> &v) {
    p.read_mapping<std::optional<Member>>(v, [](minijson::Parser &p2, Member& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void from_json(minijson::Parser &p, Member &v) {
    p.read_mapping<Member>(v, [](minijson::Parser &p2, Member& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void fill_struct(minijson::Parser &p, Definition& target, std::string_view key) {
    if (key == "name") {
        from_json(p, target.name);
    }
    else if (key == "members") {
        from_json(p, target.members);
    }
    else {
        throw minijson::BadJson(minijson::concat("unexpected key in struct of type Definition: ", std::string(key)));
    }
}

void from_json(minijson::Parser &p, std::optional<Definition> &v) {
    p.read_mapping<std::optional<Definition>>(v, [](minijson::Parser &p2, Definition& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void from_json(minijson::Parser &p, Definition &v) {
    p.read_mapping<Definition>(v, [](minijson::Parser &p2, Definition& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void fill_struct(minijson::Parser &p, DefinitionStore& target, std::string_view key) {
    if (key == "ns") {
        from_json(p, target.ns);
    }
    else if (key == "types") {
        from_json(p, target.types);
    }
    else {
        throw minijson::BadJson(minijson::concat("unexpected key in struct of type DefinitionStore: ", std::string(key)));
    }
}

void from_json(minijson::Parser &p, std::optional<DefinitionStore> &v) {
    p.read_mapping<std::optional<DefinitionStore>>(v, [](minijson::Parser &p2, DefinitionStore& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
}

void from_json(minijson::Parser &p, DefinitionStore &v) {
    p.read_mapping<DefinitionStore>(v, [](minijson::Parser &p2, DefinitionStore& target, std::string_view key) {
        fill_struct(p2, target, key);
    });
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

    out << " }";
}

void from_json(std::istream& in, TemplateParameter &v) {
    minijson::Parser parser(&in);
    from_json(parser, v);
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

    out << " }";
}

void from_json(std::istream& in, Member &v) {
    minijson::Parser parser(&in);
    from_json(parser, v);
}

void to_json(std::ostream& out, const Definition &v) {
    out << "{ ";
    out << std::quoted("name") << ": ";
    to_json(out, v.name);
    out << ", ";
    out << std::quoted("members") << ": ";
    to_json(out, v.members);

    out << " }";
}

void from_json(std::istream& in, Definition &v) {
    minijson::Parser parser(&in);
    from_json(parser, v);
}

void to_json(std::ostream& out, const DefinitionStore &v) {
    out << "{ ";
    out << std::quoted("ns") << ": ";
    to_json(out, v.ns);
    out << ", ";
    out << std::quoted("types") << ": ";
    to_json(out, v.types);

    out << " }";
}

void from_json(std::istream& in, DefinitionStore &v) {
    minijson::Parser parser(&in);
    from_json(parser, v);
}

namespace {
// start minijson_definitions.cpp.inja

namespace minijson { // definitions

std::string to_string(start_mapping_token) {
    return "{";
}

std::string to_string(end_mapping_token) {
    return "}";
}

std::string to_string(start_sequence_token) {
    return "[";
}

std::string to_string(end_sequence_token) {
    return "]";
}

std::string to_string(separator_token) {
    return ",";
}

std::string to_string(mapper_token) {
    return ":";
}

std::string to_string(const int_token &v) {
    return v.value.empty() ? "integer" : concat("(integer) ", v.value);
}

std::string to_string(const float_token &v) {
    return v.value.empty() ? "floating point" : concat("(floating point) ", v.value);
}

std::string to_string(const string_token &v) {
    return v.value.empty() ? "string" : concat("(string) ", v.value);
}

std::string to_string(true_token) {
    return "(bool) true";
}

std::string to_string(false_token) {
    return "(bool) false";
}

std::string to_string(null_token) {
    return "(null) null";
}

std::string to_string(eof_token) {
    return "(eof)";
}

std::string to_string(const token &t) {
    return std::visit([](auto&& item) {
        return to_string(item);
    }, t);
}

std::string concat() {
    return "";
}

template <typename Head, typename... Tail>
std::string concat(const Head &head, Tail&&... tail) {
    using std::to_string; 

    if constexpr (std::is_convertible_v<Head, std::string>) {
        return std::string(head) + concat(std::forward<Tail>(tail)...);
    } else if constexpr (std::is_same_v<std::decay_t<Head>, char>) {
        return std::string(1, head) + concat(std::forward<Tail>(tail)...);
    } else {
        return to_string(head) + concat(std::forward<Tail>(tail)...);
    }
}   

template <typename T>
constexpr bool match_token(const token& t) noexcept {
    return std::holds_alternative<T>(t);
}

template <typename T>
void check_token(const token& t) {
    if(!match_token<T>(t)) {
        throw BadJson(concat("unexpected token, expected ", T{}, ", got ", t));
    }
}

Tokenizer::Tokenizer(std::istream* stream)
    : d_stream(stream) {
    assert(d_stream);
    advance();
}

template <typename T, typename F>
void Parser::read_mapping(std::enable_if_t<!is_optional_v<T>, T> &target, F&& consume) {
    check_token<start_mapping_token>(d_tok.current());
    d_tok.advance();

    read_members(target, std::forward<F>(consume));

    check_token<end_mapping_token>(d_tok.current());
    d_tok.advance();
}

template <typename T, typename F>
void Parser::read_mapping(std::enable_if_t<is_optional_v<T>, T> &target, F&& consume) {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        target.reset();
    } else {
        target = typename T::value_type{};
        read_mapping<typename T::value_type>(*target, std::forward<F>(consume));
    }
}

template <typename T, typename F>
void Parser::read_sequence(std::enable_if_t<!is_optional_v<T>, T> &target, F&& consume) {
    check_token<start_sequence_token>(d_tok.current());
    d_tok.advance();

    target.clear();
    read_elements(target, std::forward<F>(consume));

    check_token<end_sequence_token>(d_tok.current());
    d_tok.advance();
}

template <typename T, typename F>
void Parser::read_sequence(std::enable_if_t<is_optional_v<T>, T> &target, F&& consume) {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        target.reset();
    } else {
        target = typename T::value_type{};
        read_sequence<typename T::value_type>(*target, std::forward<F>(consume));
    }
}

bool Parser::is_eof() const noexcept {
    return match_token<eof_token>(d_tok.current());
}

template <typename T, typename F>
void Parser::read_members(T& target, F&& consume) {
    while(match_token<string_token>(d_tok.current())) {
        read_kvpair(target, std::forward<F>(consume));

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
    std::string key = std::get<string_token>(d_tok.current()).value;

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
void extract(const token& tok, T &target) {
    std::istringstream istream(std::get<Tok>(tok).value);
    istream >> target;
}

template <typename T>
void read_actual_scalar(const token& tok, std::enable_if_t<std::is_same_v<bool, T>, T> &target) {
    if(match_token<true_token>(tok)) {
        target = true;
    } else if(match_token<false_token>(tok)) {
        target = false;
    } else {
        throw BadJson(concat("expected either true or false, got ", tok));
    }
}

template <typename T>
void read_actual_scalar(const token& tok, std::enable_if_t<std::is_floating_point_v<T>, T> &target) {
    if(match_token<float_token>(tok)) {
        extract<T, float_token>(tok, target);
    } else if(match_token<int_token>(tok)) {
        extract<T, int_token>(tok, target);
    } else {
        throw BadJson(concat("expected a number, got ", tok));
    }
}

template <typename T>
void read_actual_scalar(const token& tok, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>, T> &target) {
    check_token<int_token>(tok);
    extract<T, int_token>(tok, target);
}

template <typename T>
void read_actual_scalar(const token& tok, std::enable_if_t<std::is_same_v<std::string, T>, T> &target) {
    // other scalar types can easily be converted into strings
    if (match_token<string_token>(tok)) {
        target = std::get<string_token>(tok).value;
    } else if (match_token<true_token>(tok)) {
        target = "true";
    } else if (match_token<false_token>(tok)) {
        target = "false";
    } else if (match_token<null_token>(tok)) {
        target = "null";
    } else if (match_token<int_token>(tok)) {
        target = std::get<int_token>(tok).value;
    } else if (match_token<float_token>(tok)) {
        target = std::get<float_token>(tok).value;
    } else {
        check_token<string_token>(tok);
    }
}

template <typename T>
void Parser::read_scalar(std::enable_if_t<!is_optional_v<T>, T> &target) {
    read_actual_scalar<T>(d_tok.current(), target);
    d_tok.advance();
}

template <typename T>
void Parser::read_scalar(std::enable_if_t<is_optional_v<T>, T> &target) {
    if(match_token<null_token>(d_tok.current())) {
        d_tok.advance();
        target.reset();
    } else {
        target = typename T::value_type{};
        return read_scalar<typename T::value_type>(*target);
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
            throw BadJson(concat("unexpected char in literal, expected ", e, ", got ", (char)c));
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
            throw BadJson(concat("expected hex digit in utf8-code: ", char(c)));

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
            throw BadJson(concat("unexpected token: ", char(c)));
        }
    } else {
        d_current = eof_token{};
    }
    return current();
}

} // namespace minijson

// end minijson_definitions.cpp.inja

} // anonymous namespace
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
