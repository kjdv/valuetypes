#include <cxxopts.hpp>
#include <filesystem>
#include <generate.h>
#include <iostream>

int main(int argc, char** argv) {
    cxxopts::Options options("valuetypes", "C++ code generator for value types");
    options.add_options()("o,output", "Output directory", cxxopts::value<std::string>()->default_value(std::filesystem::relative(std::filesystem::current_path())))("f,filename", "Base filename (without extention) for the generated files", cxxopts::value<std::string>()->default_value("valuetypes"))("c,cmake", "Generate CMakeLists.txt")("input", "Input file containing type definitions", cxxopts::value<std::string>())("h,help", "Print help message");
    options.parse_positional({"input"});
    options.positional_help("input.yaml");

    auto results = options.parse(argc, argv);

    if(results.count("help")) {
        std::cout << options.help() << '\n';
        return 0;
    }

    if(!results.count("input")) {
        std::cerr << "No input file provided!\n";
        std::cerr << options.help() << '\n';
        return 1;
    }

    valuetypes::options generate_options{
        static_cast<bool>(results.count("cmake")),
        results["input"].as<std::string>(),
        results["output"].as<std::string>(),
        results["filename"].as<std::string>()};

    valuetypes::generate(generate_options);

    return 0;
}
