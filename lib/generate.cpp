#include "generate.h"
#include "definition.h"
#include "render.h"
#include <fstream>
#include <kjson/json.hh>
#include <string>
#include <string_view>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;

void generate(const options& opts) {
    fs::create_directories(opts.output_dir);

    auto doc = [&] {
        ifstream file(opts.input_file);
        return kjson::load(file);
    }()
                   .expect("could not input file, invalid json?");
    auto defs = load(doc);

    render(defs, opts);
}

} // namespace valuetypes
