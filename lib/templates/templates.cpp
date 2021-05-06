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

    include_template(env, "minijson_declarations", minijson_declarations());
    include_template(env, "minijson_definitions", minijson_definitions());

    env.set_search_included_templates_in_files(false);
    return env;
}

} // namespace templates
} // namespace valuetypes
