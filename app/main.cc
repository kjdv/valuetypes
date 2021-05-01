#include <iostream>
#include <internal_sample.hh>
#include <cxxopts.hpp>
#include <filesystem>

int main(int argc, char **argv) {
    cxxopts::Options options("valuetypes", "C++ code generator for value types");
    options.add_options()
            ("o,output", "Output directory",
                cxxopts::value<std::string>()->default_value(
                 std::filesystem::relative(std::filesystem::current_path())))
            ("c,cmake", "Generate CMakeLists.txt")
            ("input", "Input file containing type definitions",
                cxxopts::value<std::string>())
            ("h,help", "Print help message");
    options.parse_positional({"input"});
    options.positional_help("input.yaml");


    auto results = options.parse(argc, argv);

    if (results.count("help")) {
        std::cout << options.help() << '\n';
        return 0;
    }

    if (!results.count("input")) {
         std::cerr << "No input file provided!\n";
         std::cerr << options.help() << '\n';
         return 1;
    }

    bool use_cmake = results.count("cmake");
    std::filesystem::path output_dir = results["output"].as<std::string>();
    std::filesystem::path input_file = results["input"].as<std::string>();

    std::cout << use_cmake << ' ' << output_dir << ' ' << input_file << '\n';
    std::cout << valuetypes::private_function() << '\n';
    return 0;
}
