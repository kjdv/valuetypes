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

bool operator==(const Variants &a, const Variants &b) noexcept {
    return
        std::tie(a.v) ==
        std::tie(b.v);
}

bool operator!=(const Variants &a, const Variants &b) noexcept {
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

bool operator<(const Variants &a, const Variants &b) noexcept {
    return
        std::tie(a.v) <
        std::tie(b.v);
}

bool operator<=(const Variants &a, const Variants &b) noexcept {
    return !(b < a);
}

bool operator>(const Variants &a, const Variants &b) noexcept {
    return b < a;
}

bool operator>=(const Variants &a, const Variants &b) noexcept {
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
void member(std::istream &input, Nested &target);
void member(std::istream &input, Compound &target);
void member(std::istream &input, OptionalVectors &target);
void member(std::istream &input, VectorTo &target);
void member(std::istream &input, Variants &target);
struct Variants_v {
    std::variant<int, std::string, std::optional<Nested>>& base;
};
void member(std::istream& input, Variants_v& target);
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

void member(std::istream &input, Nested &target) {
    auto key = extract_key(input);
    if(key == "s") {
        element(input, target.s);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, Compound &target) {
    auto key = extract_key(input);
    if(key == "a") {
        element(input, target.a);
    } 
    else if(key == "b") {
        element(input, target.b);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, OptionalVectors &target) {
    auto key = extract_key(input);
    if(key == "v") {
        element(input, target.v);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, VectorTo &target) {
    auto key = extract_key(input);
    if(key == "v") {
        element(input, target.v);
    } 
    else {
        sink s;
        element(input, s);
    }
}
void member(std::istream &input, Variants &target) {
    auto key = extract_key(input);
    if(key == "v") {
        Variants_v t{target.v};
        element(input, t);
    } 
    else {
        sink s;
        element(input, s);
    }
}

void member(std::istream& input, Variants_v& target) {
    auto key = extract_key(input);
    if(key == "int") {
        target.base.emplace<int>();
        element(input, std::get<int>(target.base));
    }
    else if(key == "custom_str") {
        target.base.emplace<std::string>();
        element(input, std::get<std::string>(target.base));
    }
    else if(key == "std::optional<Nested>") {
        target.base.emplace<std::optional<Nested>>();
        element(input, std::get<std::optional<Nested>>(target.base));
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

void to_json(std::ostream& out, const Nested &v) {
    out << "{ ";
    out << std::quoted("s") << ": ";
    to_json(out, v.s);
    out << '}';
}

void from_json(std::istream& in, Nested &v) {
    json::value(in, v);
}

void to_json(std::ostream& out, const Compound &v) {
    out << "{ ";
    out << std::quoted("a") << ": ";
    to_json(out, v.a);
    out << ", ";
    out << std::quoted("b") << ": ";
    to_json(out, v.b);
    out << '}';
}

void from_json(std::istream& in, Compound &v) {
    json::value(in, v);
}

void to_json(std::ostream& out, const OptionalVectors &v) {
    out << "{ ";
    out << std::quoted("v") << ": ";
    to_json(out, v.v);
    out << '}';
}

void from_json(std::istream& in, OptionalVectors &v) {
    json::value(in, v);
}

void to_json(std::ostream& out, const VectorTo &v) {
    out << "{ ";
    out << std::quoted("v") << ": ";
    to_json(out, v.v);
    out << '}';
}

void from_json(std::istream& in, VectorTo &v) {
    json::value(in, v);
}

void to_json(std::ostream& out, const Variants &v) {
    out << "{ ";
    out << std::quoted("v") << ": ";
    out << "{ ";
    std::visit([&out](auto&& item) {
        using T = std::decay_t<decltype(item)>;

        if constexpr (std::is_same_v<T, int>) {
            out << std::quoted("int") << ": ";
            to_json(out, item);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            out << std::quoted("custom_str") << ": ";
            to_json(out, item);
        }
        else if constexpr (std::is_same_v<T, std::optional<Nested>>) {
            out << std::quoted("std::optional<Nested>") << ": ";
            to_json(out, item);
        }
    }, v.v);
    out << '}';
    out << '}';
}

void from_json(std::istream& in, Variants &v) {
    json::value(in, v);
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

std::ostream &operator<<(std::ostream& out, const sp::Variants &v) {
    sp::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, sp::Variants &v) {
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

std::size_t hash<sp::Variants>::operator()(const sp::Variants &v) const noexcept {
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

void swap(sp::Variants &a, sp::Variants &b) noexcept {
    swap(a.v, b.v);
}

} // namespace std

// end swap_definitions.cpp.inja
