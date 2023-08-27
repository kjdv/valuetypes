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

char next_token(std::istream& input) {
    // return the next non-whitespace char
    char c = 0;
    while(input.get(c)) {
        if(!isspace(c))
            return c;
    }
    return std::char_traits<char>::eof();
}

char peek(std::istream& input) {
    while(true) {
        char p = input.peek();
        if(!input) {
            throw json_error("peek() failed, unbufferd input?");
        }
        if(!isspace(p)) // skip whitespace
            return p;
        input.get();
    }
}

void extract_literal(std::istream& input, std::string_view l) {
    for(char e : l) {
        int c = input.get();
        if(c != e) {
            throw json_error(std::string("expected literal '") + std::string(l) + "', mismatch on char " + std::string(1, c));
        }
    }
}

std::string extract_string(std::istream& input) {
    std::string value;
    if(peek(input) == '"') {
        if(!(input >> std::quoted(value))) {
            throw json_error("failed to extract string");
        }
    } else {
        // read until the next ws or special char, interpret as string
        // this allows the qol of not always having to quote strings
        char c;
        while (input && (c = input.peek()) && (std::isalnum(c) || c == '+' || c == '-' || c == '.')) {
            value += c;
            input.get();
        }

        if (value.empty()) {
            throw json_error("failed to extract string");
        }
    }
    return value;
}

template <typename T>
T extract_value(std::istream& input) {
    if constexpr(std::is_same_v<T, std::string>) {
        return extract_string(input);
    } else {
        static_assert(std::is_arithmetic_v<T>, "type decution failed");
        T v;
        if(!(input >> v)) {
            throw json_error("could not extract number");
        }
        return v;
    }
}


void expect(char e, char actual) {
    if(actual != e) {
        throw json_error(std::string("expected '") + e + "', found '" + actual + "'");
    }
}

char expect_and_consume(std::istream& input, char e) {
    auto t = next_token(input);
    expect(e, t);
    return t;
}

struct sink {};

template <typename T>
void value(std::istream& input, T& target);

template <typename T>
void value(std::istream& input, sink target);

template <typename T>
void object(std::istream& input, T& target);

template <typename T>
void members(std::istream& input, T& target);

template <typename T>
void member(std::istream& input, T& target);

template <typename T>
void array(std::istream& input, T& target);

template <typename T>
void element(std::istream& input, T& target);

template <typename T>
void elements(std::istream& input, T& target);

template <typename T>
void value(std::istream& input, T& target) {
    if constexpr(is_optional_v<T>) {
        if(peek(input) == 'n' /* ull */) {
            extract_literal(input, "null");
            target.reset();
        } else {
            target.emplace();
            value(input, *target);
        }
    } else if constexpr(is_vector_v<T>) {
        array(input, target);
    } else if constexpr(std::is_same_v<bool, T>) {
        if(peek(input) == 't' /* rue */) {
            extract_literal(input, "true");
            target = true;
        } else if(peek(input) == 'f' /* false */) {
            extract_literal(input, "false");
            target = false;
        } else {
            target = extract_value<bool>(input);
        }
    } else if constexpr(std::is_arithmetic_v<T> || std::is_same_v<std::string, T>) {
        target = extract_value<T>(input);
    } else {
        static_assert(std::is_class_v<T>, "type deduction failed");
        object(input, target);
    }
}

void value(std::istream& input, sink target) {
    char c = peek(input);
    switch(c) {
    case '{':
        object(input, target);
        break;
    case '[': {
        std::vector<sink> v;
        array(input, v);
        break;
    }
    case 'n':
        extract_literal(input, "null");
        break;
    case 'f':
        extract_literal(input, "false");
        break;
    case 't':
        extract_literal(input, "true");
        break;
    case '"':
        extract_string(input);
        break;
    default:
        if((c >= '0' && c <= '9') || c == '+' || c == '-' || c == ',') {
            extract_value<double>(input);
        } else {
            throw json_error(std::string("expected value, found '") + c + "'");
        }
    }
}

// forward declarations
void member(std::istream &input, TemplateParameter &target);
void member(std::istream &input, Member &target);
void member(std::istream &input, Definition &target);
void member(std::istream &input, DefinitionStore &target);
template <typename T>
void object(std::istream& input, T& target) {
    // object
    //   '{' ws '}' | '{' members '}'
    expect_and_consume(input, '{');

    if(peek(input) == '"') {
        members(input, target);
    }

    expect_and_consume(input, '}');
}

template <typename T>
void array(std::istream& input, T& target) {
    // array
    //   '[' ws ']' | '[' elements ']'

    expect_and_consume(input, '[');

    if(peek(input) != ']') {
        elements(input, target);
    }

    expect_and_consume(input, ']');
}

template <typename T>
void elements(std::istream& input, T& target) {
    // elements
    //   element | element ',' elements
    static_assert(is_vector_v<T>, "expected a vector");

    target.clear();
    while(true) {
        target.emplace_back();
        element(input, target.back());

        if(peek(input) != ',') {
            break;
        }
        next_token(input);
    }
}

template <typename T>
void element(std::istream& input, T& target) {
    // element
    //   ws value ws
    value(input, target);
}

template <typename T>
void members(std::istream& input, T& target) {
    // members
    //   member | member ',' members
    while(true) {
        member(input, target);
        if(peek(input) != ',') {
            break;
        }
        next_token(input);
    }
}

std::string extract_key(std::istream& input) {
    expect_and_consume(input, '"');
    input.unget();
    auto key = extract_string(input);
    expect_and_consume(input, ':');
    return key;
}

template <typename T>
void member(std::istream& input, T& target) {
    // member
    //   ws string ws ':' element

    extract_key(input);
    element(input, target);
}

void member(std::istream &input, TemplateParameter &target) {
    auto key = extract_key(input);
    if(key == "type") {
        element(input, target.type);
    } 
    else if(key == "optional") {
        element(input, target.optional);
    } 
    else if(key == "name") {
        element(input, target.name);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, Member &target) {
    auto key = extract_key(input);
    if(key == "name") {
        element(input, target.name);
    } 
    else if(key == "type") {
        element(input, target.type);
    } 
    else if(key == "default_value") {
        element(input, target.default_value);
    } 
    else if(key == "optional") {
        element(input, target.optional);
    } 
    else if(key == "value_type") {
        element(input, target.value_type);
    } 
    else if(key == "value_types") {
        element(input, target.value_types);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, Definition &target) {
    auto key = extract_key(input);
    if(key == "name") {
        element(input, target.name);
    } 
    else if(key == "members") {
        element(input, target.members);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, DefinitionStore &target) {
    auto key = extract_key(input);
    if(key == "ns") {
        element(input, target.ns);
    } 
    else if(key == "types") {
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
    json::value(in, v);
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
    json::value(in, v);
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
    json::value(in, v);
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
    json::value(in, v);
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
