#include "generate.hh"
#include "definition.hh"
#include "render.hh"
#include <fstream>
#include <kyaml/kyaml.hh>
#include <string>
#include <string_view>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;

namespace {

unique_ptr<const kyaml::document> load(const fs::path& input_yaml) {
    ifstream      file(input_yaml);
    kyaml::parser p(file);
    return p.parse();
}

} // namespace

void generate(const options& opts) {
    fs::create_directories(opts.output_dir);

    auto doc  = load(opts.input_file);
    auto defs = DefinitionStore(*doc);

    render(defs, opts);
}

} // namespace valuetypes
