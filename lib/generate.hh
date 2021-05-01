#pragma once

#include <filesystem>

namespace valuetypes {

struct options {
    bool cmake{false};
    std::filesystem::path input_file;
    std::filesystem::path output_dir;
};

void generate(const options& opts);

}
