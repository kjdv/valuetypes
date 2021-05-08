#include "generate.h"
#include "definitions/valuetypes.h"
#include "render.h"
#include <fstream>
#include <string>
#include <string_view>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;

void generate(const Options& opts) {
    fs::create_directories(opts.output_dir);

    auto defs = [&] {
        ifstream        file(opts.input_file);
        DefinitionStore ds;
        from_json(file, ds);
        return ds;
    }();
    auto vars = transform(move(defs));

    render(move(vars), opts);
}

} // namespace valuetypes
