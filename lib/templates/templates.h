#pragma once

#include <inja/inja.hpp>
#include <string_view>

namespace valuetypes {
namespace templates {

std::string_view header() noexcept;
std::string_view source() noexcept;

inja::Environment make_env();

std::string_view hash_declarations() noexcept;
std::string_view hash_definitions() noexcept;

std::string_view minijson_declarations() noexcept;
std::string_view minijson_definitions() noexcept;

} // namespace templates
} // namespace valuetypes
