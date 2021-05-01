#include "generate.hh"
#include <string>
#include <string_view>
#include <fstream>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;

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

}

void generate(const options &opts)
{
    fs::create_directories(opts.output_dir);

    auto header_filename = output_file(opts.output_dir, opts.base_filename, ".hh");
    auto source_filename = output_file(opts.output_dir, opts.base_filename, ".cc");

    write(header_filename, "#pragma once\n");
    write(source_filename, "#include \"valuetypes.hh\"\n");
}

}
