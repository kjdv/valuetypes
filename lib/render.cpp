#include "render.h"
#include "templates/templates.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include <iostream>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;
using json   = nlohmann::json;

namespace {

fs::path output_file(const fs::path& dir, const fs::path& base_filename, string_view extension) {
    fs::path cp(dir);
    fs::path filename = base_filename;
    filename += extension;
    cp /= filename;
    return cp;
}

void render(const fs::path& p, inja::Environment& env, string_view tmpl, const json& data) {
    ofstream file(p);
    file << env.render(tmpl, data);
}

json member_to_json(const Member& m) {
    json d;
    d["name"] = m.name;

    auto type = [&] {
        auto base = m.type;
        if(m.value_type) {
            base += string("<") + m.value_type->type + ">";
        }
        if(m.optional) {
            base = string("std::optional<") + base + ">";
        }
        return base;
    }();

    d["type"] = type;

    if(m.default_value) {
        d["default_value"] = *m.default_value;
    } else {
        d["default_value"] = nullptr;
    }

    return d;
}

json defintion_to_json(const Definition& d) {
    json j;
    j["name"] = d.name;

    vector<json> members;
    members.reserve(d.members.size());
    transform(d.members.begin(), d.members.end(), back_inserter(members), member_to_json);

    j["members"] = move(members);

    return j;
}

json defstore_to_json(const DefinitionStore& ds) {
    vector<json> defs;
    defs.reserve(ds.typedefs.size());

    transform(ds.typedefs.begin(), ds.typedefs.end(), back_inserter(defs), [&](auto&& item) {
        auto d = defintion_to_json(item);
        if(ds.namespace_) {
            d["namespace_name"] = (*ds.namespace_) + "::" + item.name;
        } else {
            d["namespace_name"] = item.name;
        }
        return d;
    });

    json j;
    j["typedefs"] = move(defs);

    if(ds.namespace_) {
        j["namespace"] = *ds.namespace_;
    } else {
        j["namespace"] = nullptr;
    }

    return j;
}

json opts_to_json(const options& opts) {
    json d;
    d["base_filename"] = opts.base_filename;
    return d;
}

} // namespace

void render(const DefinitionStore& ds, const options& opts) {
    auto header_filename = output_file(opts.output_dir, opts.base_filename, ".h");
    auto source_filename = output_file(opts.output_dir, opts.base_filename, ".cpp");

    json data       = defstore_to_json(ds);
    data["options"] = opts_to_json(opts);

    auto env = templates::make_env();

    render(header_filename, env, templates::header(), data);
    render(source_filename, env, templates::source(), data);
}

} // namespace valuetypes
