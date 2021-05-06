#pragma once

#include <inja/inja.hpp>
#include <string_view>

namespace valuetypes {
namespace templates {

std::string_view header() noexcept;
std::string_view source() noexcept;

inja::Environment make_env();

} // namespace templates
} // namespace valuetypes
