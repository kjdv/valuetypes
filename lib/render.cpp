#include "render.h"
#include "templates/templates.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
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

void render(const fs::path& p, inja::Environment& env, string_view tmpl, const json& data) {
    ofstream file(p);
    file << env.render(tmpl, data);
}

Variables opts_to_vars(const Options& opts) {
    Variables d;

    const regex libname_regex(R"([a-zA-Z][a-zA-Z\-_]*)");

    string libname  = fs::absolute(opts.output_dir).filename();
    bool   is_valid = regex_match(libname, libname_regex);
    if(!is_valid) {
        libname  = fs::absolute(opts.output_dir).parent_path().filename();
        is_valid = regex_match(libname, libname_regex);
    }

    d["library_name"]    = is_valid ? libname : string(opts.base_filename);
    d["base_filename"]   = opts.base_filename;
    d["permissive_json"] = opts.permissive_json;
    d["json"] = opts.json;

    return d;
}

} // namespace

void render(Variables vars, const Options& opts) {
    auto header_filename = output_file(opts.output_dir, opts.base_filename, ".h");
    auto source_filename = output_file(opts.output_dir, opts.base_filename, ".cpp");

    vars["options"] = opts_to_vars(opts);

    auto env = templates::make_env();

    render(header_filename, env, templates::header(), vars);
    render(source_filename, env, templates::source(), vars);

    if(opts.cmake) {
        render(output_file(opts.output_dir, "CMakeLists", ".txt"), env, templates::cmakelists(), vars);
    }
}

} // namespace valuetypes
