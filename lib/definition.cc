#include "definition.hh"
#include <algorithm>
#include <string>

namespace valuetypes {

using namespace std;

namespace {

Member make_basic(string_view name, string_view type, string_view default_value) {
    return Member{
        string(name),
        string(type),
        string(default_value)};
}

Member make_double(string_view name, string_view default_value = "0.0") {
    return make_basic(name, "double", default_value);
}

const string& extract(const kyaml::mapping& m, string_view key) {
    return m.get(std::string(key)).as_scalar().get();
}

Member from_yaml(const kyaml::node& n) {
    auto& m    = n.as_mapping();
    auto& type = extract(m, "type");

    if(type == "double") {
        return make_double(extract(m, "name"));
    } else {
        throw BadDefinition("no such type");
    }
}

} // namespace

DefinitionStore load(const kyaml::document& doc) {
    vector<Definition> defs;
    defs.reserve(doc.as_sequence().size());

    transform(doc.as_sequence().begin(), doc.as_sequence().end(), back_inserter(defs), [](auto&& d) {
        auto& m = d->as_mapping();

        auto& name = extract(m, "name");
        auto& mseq = m.get("members").as_sequence();

        vector<Member> members;
        members.reserve(mseq.size());
        transform(mseq.begin(), mseq.end(), back_inserter(members), [](auto&& item) { return from_yaml(*item); });

        return Definition{name, move(members)};
    });

    return DefinitionStore{move(defs)};
}

} // namespace valuetypes
