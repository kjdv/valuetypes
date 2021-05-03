#include "definition.h"
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

Member make_bool(string_view name, string_view default_value = "false") {
    return make_basic(name, "bool", default_value);
}

string_view extract(const composite::mapping& m, string_view key) {
    return m.at(string(key)).as<string>();
}

Member from_composite(const composite::composite& n) {
    auto& m    = n.as<composite::mapping>();
    auto  type = extract(m, "type");
    auto name = extract(m, "name");

    if(type == "bool") {
        return make_bool(name);
    } else if(type == "double") {
        return make_double(name);
    } else {
        throw BadDefinition("no such type");
    }
}

} // namespace

DefinitionStore load(const kjson::document& doc) {
    auto&            as_map = doc.as<composite::mapping>();
    optional<string> ns;
    if(auto it = as_map.find("namespace"); it != as_map.end()) {
        ns = it->second.as<string>();
    }

    vector<Definition> defs;
    auto&              ts = as_map.at("types").as<composite::sequence>();
    defs.reserve(ts.size());

    transform(ts.begin(), ts.end(), back_inserter(defs), [](const composite::composite& d) {
        auto& m = d.as<composite::mapping>();

        auto  name = extract(m, "name");
        auto& mseq = m.at("members").as<composite::sequence>();

        vector<Member> members;
        members.reserve(mseq.size());
        transform(mseq.begin(), mseq.end(), back_inserter(members), from_composite);

        return Definition{string(name), move(members)};
    });

    return DefinitionStore{ns, move(defs)};
}

} // namespace valuetypes
