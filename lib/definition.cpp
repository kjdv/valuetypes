#include "definition.h"
#include <algorithm>
#include <string>
#include <unordered_map>

namespace valuetypes {

using namespace std;

namespace {

const unordered_map<string_view, string_view> int_like = {
    {"int", "int"},
    {"uint", "unsigned int"},
    {"int8", "int8_t"},
    {"uint8", "uint8_t"},
    {"int16", "int16_t"},
    {"uint16", "uint16_t"},
    {"int32", "int32_t"},
    {"uint32", "uint32_t"},
    {"int64", "int64_t"},
    {"uint64", "uint64_t"}};

const unordered_map<string_view, string_view> float_like = {
    {"float", "float"},
    {"double", "double"}};

Member make_basic(string_view name, string_view type, string_view default_value) {
    return Member{
        string(name),
        string(type),
        string(default_value)};
}

Member make_bool(string_view name, string_view default_value = "false") {
    return make_basic(name, "bool", default_value);
}

Member make_int_like(string_view name, string_view type, string_view default_value = "0") {
    return make_basic(name, type, default_value);
}

Member make_float_like(string_view name, string_view type, string_view default_value = "0.0") {
    return make_basic(name, type, default_value);
}

string_view extract(const composite::mapping& m, string_view key) {
    return m.at(string(key)).as<string>();
}

Member from_composite(const composite::composite& n) {
    auto& m           = n.as<composite::mapping>();
    auto  type        = extract(m, "type");
    auto  name        = extract(m, "name");
    bool  is_optional = [&] {
        auto it = m.find("optional");
        return it != m.end() && it->second.as<bool>();
    }();

    Member member = [&] {
        if(type == "bool") {
            return make_bool(name);
        } else if(auto it = int_like.find(type); it != int_like.end()) {
            return make_int_like(name, it->second);
        } else if(auto it = float_like.find(type); it != float_like.end()) {
            return make_float_like(name, it->second);
        } else if(type == "string") {
            return Member{string(name), "std::string", optional<string>{}};
        } else {
            throw BadDefinition("no such type");
        }
    }();
    if(is_optional) {
        member.optional = true;
        member.default_value.reset(); // todo: nicify
    }

    return member;
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
