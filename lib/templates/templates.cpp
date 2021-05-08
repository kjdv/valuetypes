#include "templates.h"

namespace valuetypes {
namespace templates {

namespace {

void include_template(inja::Environment& env, std::string_view name, std::string_view content) {
    auto tmpl = env.parse(content);
    env.include_template(std::string(name), tmpl);
}

} // namespace

inja::Environment make_env() {
    inja::Environment env;
    env.set_search_included_templates_in_files(false);

    include_template(env, "minijson_declarations", minijson_declarations());
    include_template(env, "minijson_definitions", minijson_definitions());
    include_template(env, "comparison_declarations", comparison_declarations());
    include_template(env, "comparison_definitions", comparison_definitions());
    include_template(env, "equality_declarations", equality_declarations());
    include_template(env, "equality_definitions", equality_definitions());
    include_template(env, "hash_declarations", hash_declarations());
    include_template(env, "hash_definitions", hash_definitions());
    include_template(env, "iostream_declarations", iostream_declarations());
    include_template(env, "iostream_definitions", iostream_definitions());

    return env;
}

std::string_view cmakelists() noexcept
{
    return "add_library({{ options.library_name }} {{ options.base_filename }}.h {{ options.base_filename }}.cpp)\n";
}

} // namespace templates
} // namespace valuetypes
