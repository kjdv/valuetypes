#include "templates.hh"

const std::string valuetypes::source = R"raw(#include "{{ options.base_filename }}.hh"
#include <tuple>

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
## endfor

{% if namespace %}}{% endif %}
)raw";
