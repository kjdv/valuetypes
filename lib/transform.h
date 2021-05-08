#pragma once

#include <definitions/valuetypes.h>
#include <nlohmann/json.hpp>

namespace valuetypes {

using Variables = nlohmann::json;

class ValidationError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Variables transform(DefinitionStore doc);

} // namespace valuetypes
