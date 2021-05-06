#include "templates.h"

namespace valuetypes {
namespace templates {

inja::Environment make_env() {
    inja::Environment env;
    env.set_search_included_templates_in_files(false);
    return env;
}

} // namespace templates
} // namespace valuetypes
