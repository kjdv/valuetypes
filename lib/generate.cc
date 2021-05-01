#include "generate.hh"
#include <string>
#include <string_view>
#include <fstream>

namespace valuetypes {

using namespace std;
namespace fs = std::filesystem;

namespace {

fs::path output_file(const fs::path& dir, string_view filename) {
    fs::path cp(dir);
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
    const string name = "point";

    fs::create_directories(opts.output_dir);

    auto header_filename = output_file(opts.output_dir, name + ".hh");
    auto source_filename = output_file(opts.output_dir, name + ".cc");

    write(header_filename, "#pragma once\n");
    write(source_filename, "#include \"point.hh\"\n");
}

}
