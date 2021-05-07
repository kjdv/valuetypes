#pragma once

#include <kjson/json.hh>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace valuetypes {

class BadDefinition : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct AnonymousType {
    std::string type;
    bool        optional{false};
};

struct Member {
    std::string                  name;
    std::string                  type;
    std::optional<std::string>   default_value{};
    bool                         optional{false};
    std::optional<AnonymousType> value_type{}; // only applicable for vectors
};

struct Definition {
    std::string         name;
    std::vector<Member> members;
};

struct DefinitionStore {
    std::optional<std::string> namespace_;
    std::vector<Definition>    typedefs;
};

DefinitionStore load(const kjson::document& doc);

} // namespace valuetypes
