#include "templates.h"

const std::string valuetypes::header = R"raw(
#pragma once

#include <functional>

{% if namespace %}namespace {{ namespace }} { {% endif %}
## for typedef in typedefs

struct {{ typedef.name }} {
## for member in typedef.members
    {{ member.type }} {{ member.name }}{% if member.default_value %}{ {{ member.default_value }} }{% endif %};
## endfor
};

bool operator==(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator!=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;

bool operator<(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator<=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator>(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
bool operator>=(const {{ typedef.name }} &a, const {{ typedef.name}} &b) noexcept;
## endfor

{% if namespace %}} // namespace {{ namespace }}{% endif %}

namespace std {

## for typedef in typedefs
template<>
struct hash<{{typedef.namespace_name}}> {
    std::size_t operator()(const {{typedef.namespace_name}} &v) const noexcept;
};

## endfor
} // namespace std
)raw";
