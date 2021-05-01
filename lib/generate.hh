#pragma once

#include <filesystem>
#include <string>

namespace valuetypes {

struct options {
    bool cmake{false};
    std::filesystem::path input_file;
    std::filesystem::path output_dir;
    std::filesystem::path base_filename;
};

void generate(const options& opts);

}
