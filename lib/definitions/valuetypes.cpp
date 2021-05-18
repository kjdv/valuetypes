#include "valuetypes.h"
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <algorithm>
#include <kjson/json.hh>
#include <composite/make.hh>

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

kjson::document to_kjson(const TemplateParameter &v);
void from_kjson(const kjson::document &doc, TemplateParameter &target);

kjson::document to_kjson(const Member &v);
void from_kjson(const kjson::document &doc, Member &target);

kjson::document to_kjson(const Definition &v);
void from_kjson(const kjson::document &doc, Definition &target);

kjson::document to_kjson(const DefinitionStore &v);
void from_kjson(const kjson::document &doc, DefinitionStore &target);


template <typename T>
kjson::document to_kjson(const T &v) {
    if constexpr (is_optional_v<T>) {
        if (!v) {
            return kjson::document{};
        } else {
            return to_kjson(*v);
        }
    } else if constexpr (is_vector_v<T>) {
        auto seq = composite::sequence();
        seq.reserve(v.size());
        std::transform(v.begin(), v.end(), std::back_inserter(seq), [](auto&& item) { return to_kjson(item); });
        return kjson::document(std::move(seq));
    } else {
        return kjson::document(v);
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

kjson::document to_kjson(const TemplateParameter &v) {
    return composite::make_mapping(
        "type", to_kjson(v.type),
        "optional", to_kjson(v.optional),
        "name", to_kjson(v.name)
    );
}

void from_kjson(const kjson::document &doc, TemplateParameter &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("type"); it != m.end()) {
        from_kjson(it->second, target.type);
    }
    if (auto it = m.find("optional"); it != m.end()) {
        from_kjson(it->second, target.optional);
    }
    if (auto it = m.find("name"); it != m.end()) {
        from_kjson(it->second, target.name);
    }
}

kjson::document to_kjson(const Member &v) {
    return composite::make_mapping(
        "name", to_kjson(v.name),
        "type", to_kjson(v.type),
        "default_value", to_kjson(v.default_value),
        "optional", to_kjson(v.optional),
        "value_type", to_kjson(v.value_type),
        "value_types", to_kjson(v.value_types)
    );
}

void from_kjson(const kjson::document &doc, Member &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("name"); it != m.end()) {
        from_kjson(it->second, target.name);
    }
    if (auto it = m.find("type"); it != m.end()) {
        from_kjson(it->second, target.type);
    }
    if (auto it = m.find("default_value"); it != m.end()) {
        from_kjson(it->second, target.default_value);
    }
    if (auto it = m.find("optional"); it != m.end()) {
        from_kjson(it->second, target.optional);
    }
    if (auto it = m.find("value_type"); it != m.end()) {
        from_kjson(it->second, target.value_type);
    }
    if (auto it = m.find("value_types"); it != m.end()) {
        from_kjson(it->second, target.value_types);
    }
}

kjson::document to_kjson(const Definition &v) {
    return composite::make_mapping(
        "name", to_kjson(v.name),
        "members", to_kjson(v.members)
    );
}

void from_kjson(const kjson::document &doc, Definition &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("name"); it != m.end()) {
        from_kjson(it->second, target.name);
    }
    if (auto it = m.find("members"); it != m.end()) {
        from_kjson(it->second, target.members);
    }
}

kjson::document to_kjson(const DefinitionStore &v) {
    return composite::make_mapping(
        "ns", to_kjson(v.ns),
        "types", to_kjson(v.types)
    );
}

void from_kjson(const kjson::document &doc, DefinitionStore &target) {
    auto& m = doc.as<composite::mapping>();
    if (auto it = m.find("ns"); it != m.end()) {
        from_kjson(it->second, target.ns);
    }
    if (auto it = m.find("types"); it != m.end()) {
        from_kjson(it->second, target.types);
    }
}


} // anonymous namespace

void to_json(std::ostream& out, const TemplateParameter &v) {
    auto doc = to_kjson(v);
    kjson::dump(doc, out);
}

void from_json(std::istream& in, TemplateParameter &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const Member &v) {
    auto doc = to_kjson(v);
    kjson::dump(doc, out);
}

void from_json(std::istream& in, Member &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const Definition &v) {
    auto doc = to_kjson(v);
    kjson::dump(doc, out);
}

void from_json(std::istream& in, Definition &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
}

void to_json(std::ostream& out, const DefinitionStore &v) {
    auto doc = to_kjson(v);
    kjson::dump(doc, out);
}

void from_json(std::istream& in, DefinitionStore &v) {
    auto doc = kjson::load(in).expect("invalid json");
    from_kjson(doc, v);
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
