#include "templates.h"

const std::string valuetypes::header = R"raw(
#pragma once

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>

{% if namespace %}namespace {{ namespace }} { {% endif %}
## for typedef in typedefs

struct {{ typedef.name }} {
## for member in typedef.members
    {{ member.type }} {{ member.name }}{% if member.default_value %} { {{ member.default_value }} }{% endif %};
## endfor
};

bool operator==(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator!=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;

bool operator<(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator<=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator>(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator>=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;

void to_json(std::ostream& out, const {{ typedef.name }} &v);
void from_json(std::istream& in, {{ typedef.name }} &v);

## endfor

{% if namespace %}} // namespace {{ namespace }}{% endif %}

namespace std {

## for typedef in typedefs

ostream &operator<<(ostream& out, const {{ typedef.namespace_name }} &v);
istream &operator>>(istream& in, {{ typedef.namespace_name }} &v);

template<>
struct hash<{{typedef.namespace_name}}> {
    std::size_t operator()(const {{typedef.namespace_name}} &v) const noexcept;
};

## endfor
} // namespace std
)raw";
