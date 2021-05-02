#include "templates.h"

const std::string valuetypes::source = R"raw(#include "{{ options.base_filename }}.h"
#include <tuple>
#include <utility>

{% if namespace %}namespace {{ namespace }} { {% endif %}

using namespace std;
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