#include "valuetypes.h"
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <algorithm>
#include <kjson/json.hh>
#include <kjson/builder.hh>
#include <composite/make.hh>

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

void to_kjson(kjson::builder &builder, const Nested &v);
void from_kjson(const kjson::document &doc, Nested &target);

void to_kjson(kjson::builder &builder, const Compound &v);
void from_kjson(const kjson::document &doc, Compound &target);

void to_kjson(kjson::builder &builder, const OptionalVectors &v);
void from_kjson(const kjson::document &doc, OptionalVectors &target);

void to_kjson(kjson::builder &builder, const VectorTo &v);
void from_kjson(const kjson::document &doc, VectorTo &target);


template <typename T>
void to_kjson(kjson::builder &builder, const T &v) {
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

template <typename T>
void from_kjson(const kjson::document &doc, T &target) {
    if constexpr (is_optional_v<T>) {
        if (doc.is<composite::none>()) {
            target.reset();
        } else {
            target.emplace();
            from_kjson(doc, *target);
        }
    } else if constexpr (is_vector_v<T>) {
        using U = typename T::value_type;

        target.clear();
        auto& as_seq = doc.as<composite::sequence>();
        std::transform(as_seq.begin(), as_seq.end(), std::back_inserter(target), [](auto&& item){
            U v{};
            from_kjson(item, v);
            return v;
        });
    } else {
        target = doc.to<T>();
    }
}

void to_kjson(kjson::builder &builder, const Nested &v) {
    builder.push_mapping();
    builder.key("s");
    to_kjson(builder, v.s);
    builder.pop();
}

void from_kjson(const kjson::document &doc, Nested &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("s"); it != m.end()) {
        from_kjson(it->second, target.s);
    }
}

void to_kjson(kjson::builder &builder, const Compound &v) {
    builder.push_mapping();
    builder.key("a");
    to_kjson(builder, v.a);
    builder.key("b");
    to_kjson(builder, v.b);
    builder.pop();
}

void from_kjson(const kjson::document &doc, Compound &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("a"); it != m.end()) {
        from_kjson(it->second, target.a);
    }
    if (auto it = m.find("b"); it != m.end()) {
        from_kjson(it->second, target.b);
    }
}

void to_kjson(kjson::builder &builder, const OptionalVectors &v) {
    builder.push_mapping();
    builder.key("v");
    to_kjson(builder, v.v);
    builder.pop();
}

void from_kjson(const kjson::document &doc, OptionalVectors &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("v"); it != m.end()) {
        from_kjson(it->second, target.v);
    }
}

void to_kjson(kjson::builder &builder, const VectorTo &v) {
    builder.push_mapping();
    builder.key("v");
    to_kjson(builder, v.v);
    builder.pop();
}

void from_kjson(const kjson::document &doc, VectorTo &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("v"); it != m.end()) {
        from_kjson(it->second, target.v);
    }
}


} // anonymous namespace

void to_json(std::ostream& out, const Nested &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Nested &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const Compound &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Compound &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const OptionalVectors &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, OptionalVectors &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const VectorTo &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, VectorTo &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
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
