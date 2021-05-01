#include "templates.hh"

const std::string valuetypes::header = R"raw(
#pragma once

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
)raw";
