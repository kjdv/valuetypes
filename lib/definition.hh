#pragma once

#include <string_view>
#include <vector>
#include <string>
#include <stdexcept>
#include <optional>
#include <kyaml/node.hh>

namespace valuetypes {

class BadDefinition : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Member {
    std::string name;
    std::string type;
    std::optional<std::string> default_value;
};

struct Definition {
    std::string name;
    std::vector<Member> members;
};

class DefinitionStore {
public:
    DefinitionStore(const kyaml::document& doc);
private:
    std::vector<Definition> d_definitions;
};

}