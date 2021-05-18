#pragma once

#include <filesystem>
#include <string>

namespace valuetypes {

struct Options {
    bool                  cmake{false};
    std::filesystem::path input_file;
    std::filesystem::path output_dir;
    std::filesystem::path base_filename;
    bool                  json{false};
};

void generate(const Options& opts);

} // namespace valuetypes
