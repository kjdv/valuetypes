#pragma once

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace vt { 

struct Nested {
    std::string s {  } ;
};

struct Compound {
    Nested a {  } ;
    Nested b {  } ;
};

bool operator==(const Nested &a, const Nested &b) noexcept;
bool operator!=(const Nested &a, const Nested &b) noexcept;

bool operator==(const Compound &a, const Compound &b) noexcept;
bool operator!=(const Compound &a, const Compound &b) noexcept;


bool operator<(const Nested &a, const Nested &b) noexcept;
bool operator<=(const Nested &a, const Nested &b) noexcept;
bool operator>(const Nested &a, const Nested &b) noexcept;
bool operator>=(const Nested &a, const Nested &b) noexcept;

bool operator<(const Compound &a, const Compound &b) noexcept;
bool operator<=(const Compound &a, const Compound &b) noexcept;
bool operator>(const Compound &a, const Compound &b) noexcept;
bool operator>=(const Compound &a, const Compound &b) noexcept;


} // namespace vt

namespace vt { 

void to_json(std::ostream& out, const Nested &v);
void from_json(std::istream& in, Nested &v);

void to_json(std::ostream& out, const Compound &v);
void from_json(std::istream& in, Compound &v);


} // namespace vt

namespace std {

ostream &operator<<(ostream& out, const vt::Nested &v);
istream &operator>>(istream& in, vt::Nested &v);

ostream &operator<<(ostream& out, const vt::Compound &v);
istream &operator>>(istream& in, vt::Compound &v);

} // namespace std

namespace std {

template<>
struct hash<vt::Nested> {
    std::size_t operator()(const vt::Nested &v) const noexcept;
};

template<>
struct hash<vt::Compound> {
    std::size_t operator()(const vt::Compound &v) const noexcept;
};

} // namespace std

namespace std {

void swap(vt::Nested &a, vt::Nested &b) noexcept;

void swap(vt::Compound &a, vt::Compound &b) noexcept;

} // namespace std

