#include "render.hh"
#include "templates/templates.hh"
#include <algorithm>
#include <fstream>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <vector>

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

void write(const fs::path& p, string_view content) {
    ofstream file(p);
    file << content;
}

json member_to_json(const Member m) {
    json d;
    d["name"] = m.name;
    d["type"] = m.type;

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
    defs.reserve(ds.definitions.size());

    transform(ds.definitions.begin(), ds.definitions.end(), back_inserter(defs), defintion_to_json);

    json j;
    j["typedefs"] = move(defs);

    return j;
}

json opts_to_json(const options& opts) {
    json d;
    d["base_filename"] = opts.base_filename;
    return d;
}

} // namespace

void render(const DefinitionStore& ds, const options& opts) {
    auto header_filename = output_file(opts.output_dir, opts.base_filename, ".hh");
    auto source_filename = output_file(opts.output_dir, opts.base_filename, ".cc");

    json data       = defstore_to_json(ds);
    data["options"] = opts_to_json(opts);

    write(header_filename, inja::render(header, data));
    write(source_filename, inja::render(source, data));

    write(output_file(opts.output_dir, ".stamp", ""), "");
}

} // namespace valuetypes
