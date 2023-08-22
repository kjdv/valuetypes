#include "transform.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

const unordered_map<string_view, string_view> one_to_one = {
    {"bool", "bool"},
    {"string", "std::string"},
    {"vector", "std::vector"},
    {"variant", "std::variant"}};

template <typename T>
void fill_optional(Variables& vars, const char* key, const std::optional<T>& value) {
    if(value) {
        vars[key] = *value;
    } else {
        vars[key] = nullptr;
    }
}

string optionalize(string_view base) {
    return string("std::optional<") + string(base) + ">";
}

string maybe_optionalize(bool should, string_view base) {
    return should ? optionalize(base) : string(base);
}

string validated_type(string_view type, const unordered_set<string>& local_typedefs) {
    if(auto it = int_like.find(type); it != int_like.end()) {
        return string(it->second);
    } else if(auto it = float_like.find(type); it != float_like.end()) {
        return string(it->second);
    } else if(auto it = one_to_one.find(type); it != one_to_one.end()) {
        return string(it->second);
    } else if(auto it = local_typedefs.find(string(type)); it != local_typedefs.end()) {
        return *it;
    } else {
        throw ValidationError(string("unrecognized type: ") + string(type));
    }
};

string real_type(const TemplateParameter& member, const unordered_set<string>& local_typedefs) {
    string base = validated_type(member.type, local_typedefs);
    return maybe_optionalize(member.optional, base);
}

string real_type(const Member& member, const unordered_set<string>& local_typedefs) {
    string base = validated_type(member.type, local_typedefs);

    if(member.value_type) {
        base += string("<") + maybe_optionalize(member.value_type->optional, member.value_type->type) + ">";
    } else if(member.value_types) {
        base += string("<");

        bool first = true;
        for(auto& vt : *member.value_types) {
            if(!first) {
                base += ", ";
            }
            base += real_type(vt, local_typedefs);
            first = false;
        }
        base += ">";
    }

    return maybe_optionalize(member.optional, base);
}

Variables transform(const Member& member, const unordered_set<string>& local_typedefs) {
    Variables vars;

    vars["name"] = member.name;
    vars["type"] = real_type(member, local_typedefs);
    vars["optional"] = member.optional;

    if(member.value_types) {
        vector<Variables> vts;
        std::transform(member.value_types->begin(), member.value_types->end(), back_inserter(vts), [&](auto&& item) {
            Variables j;
            auto      n = real_type(item, local_typedefs);
            j["type"]   = n;
            if(item.name) {
                j["name"] = *item.name;
            } else {
                j["name"] = n;
            }

            return j;
        });

        vars["value_types"] = vts;
    } else {
        vars["value_types"] = nullptr;
    }

    if (member.value_type) {
        vars["value_type"] = real_type(*member.value_type, local_typedefs);
    } else {
        vars["value_type"] = nullptr;
    }

    auto default_value = member.default_value;
    if(!default_value && !member.optional) {
        if(member.type == "bool") {
            default_value = "false";
        } else if(auto it = int_like.find(member.type); it != int_like.end()) {
            default_value = "0";
        } else if(auto it = float_like.find(member.type); it != float_like.end()) {
            default_value = "0.0";
        }
    }

    if(default_value && member.type == "string") {
        ostringstream stream;
        stream << quoted(*default_value);
        default_value = stream.str();
    }

    fill_optional(vars, "default_value", default_value);

    return vars;
}

Variables transform(const Definition& def, const unordered_set<string>& local_typedefs) {
    Variables vars;
    vars["name"] = def.name;

    vector<Variables> members;
    std::transform(def.members.begin(), def.members.end(), back_inserter(members), [&](auto&& m) {
        return transform(m, local_typedefs);
    });

    vars["members"] = move(members);

    return vars;
}

} // namespace

Variables transform(DefinitionStore ds) {
    Variables vars;
    fill_optional(vars, "namespace", ds.ns);

    vector<Variables>     defs;
    unordered_set<string> local_typedefs;

    std::transform(ds.types.begin(), ds.types.end(), back_inserter(defs), [&](const Definition& def) {
        auto var = transform(def, local_typedefs);

        if(ds.ns) {
            var["namespace_name"] = (*ds.ns) + "::" + def.name;
        } else {
            var["namespace_name"] = def.name;
        }

        local_typedefs.insert(def.name);

        return var;
    });

    vars["typedefs"] = move(defs);

    return vars;
}

} // namespace valuetypes
