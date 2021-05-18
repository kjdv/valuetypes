#include <benchmark/benchmark.h>
#include <basic_types/valuetypes.h>
#include <structs/valuetypes.h>
#include <variants/valuetypes.h>
#include <sstream>
#include <type_traits>

namespace {

template <typename T>
void bm_insertion(benchmark::State &state) {
    T v{};
    std::ostringstream stream;

    for (auto _ : state) {
        stream << v;
        stream.clear();
    }
}

template <typename T>
constexpr char const *sample_json() {
    if constexpr (std::is_same_v<T, bt::BasicTypes>) {
        return R"({ "truth": false, "n": 0, "x": 0, "s": "" })";
    } else if constexpr (std::is_same_v<T, vt::Compound>) {
        return R"({ "a": { "s": "abc" }, "b": { "s": "def" } })";
    } else if constexpr (std::is_same_v<T, vt::Variants>) {
        return R"({ "v": { "std::optional<Base>": { "n": 123 } } })";
    } else {
        return nullptr;
    }
}

template <typename T>
void bm_extraction(benchmark::State &state) {
    T v{};
    std::istringstream stream(sample_json<T>());

    for (auto _ : state) {
        stream >> v;
        stream.clear();
        stream.str(sample_json<T>());
    }
}

BENCHMARK_TEMPLATE(bm_insertion, bt::BasicTypes);
BENCHMARK_TEMPLATE(bm_extraction, bt::BasicTypes);

BENCHMARK_TEMPLATE(bm_insertion, vt::Compound);
BENCHMARK_TEMPLATE(bm_extraction, vt::Compound);

BENCHMARK_TEMPLATE(bm_insertion, vt::Variants);
BENCHMARK_TEMPLATE(bm_extraction, vt::Variants);

}

BENCHMARK_MAIN();
