#include "templates.hh"

const std::string valuetypes::header = R"raw(
#pragma once

## for typedef in typedefs
struct {{ typedef.name }} {
## for member in typedef.members
    {{ member.type }} {{ member.name }}{member.default_value};
##
};
## endfor

)raw";
