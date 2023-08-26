#pragma once

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace sp { 

struct Nested {
    std::string s {  } ;
};

struct Compound {
    Nested a {  } ;
    Nested b {  } ;
};

struct OptionalVectors {
    std::optional<std::vector<std::optional<int>>> v {  } ;
};

struct VectorTo {
    std::vector<Nested> v {  } ;
};

bool operator==(const Nested &a, const Nested &b) noexcept;
bool operator!=(const Nested &a, const Nested &b) noexcept;

bool operator==(const Compound &a, const Compound &b) noexcept;
bool operator!=(const Compound &a, const Compound &b) noexcept;

bool operator==(const OptionalVectors &a, const OptionalVectors &b) noexcept;
bool operator!=(const OptionalVectors &a, const OptionalVectors &b) noexcept;

bool operator==(const VectorTo &a, const VectorTo &b) noexcept;
bool operator!=(const VectorTo &a, const VectorTo &b) noexcept;


bool operator<(const Nested &a, const Nested &b) noexcept;
bool operator<=(const Nested &a, const Nested &b) noexcept;
bool operator>(const Nested &a, const Nested &b) noexcept;
bool operator>=(const Nested &a, const Nested &b) noexcept;

bool operator<(const Compound &a, const Compound &b) noexcept;
bool operator<=(const Compound &a, const Compound &b) noexcept;
bool operator>(const Compound &a, const Compound &b) noexcept;
bool operator>=(const Compound &a, const Compound &b) noexcept;

bool operator<(const OptionalVectors &a, const OptionalVectors &b) noexcept;
bool operator<=(const OptionalVectors &a, const OptionalVectors &b) noexcept;
bool operator>(const OptionalVectors &a, const OptionalVectors &b) noexcept;
bool operator>=(const OptionalVectors &a, const OptionalVectors &b) noexcept;

bool operator<(const VectorTo &a, const VectorTo &b) noexcept;
bool operator<=(const VectorTo &a, const VectorTo &b) noexcept;
bool operator>(const VectorTo &a, const VectorTo &b) noexcept;
bool operator>=(const VectorTo &a, const VectorTo &b) noexcept;


} // namespace sp

namespace sp { 

void to_json(std::ostream& out, const Nested &v);
void from_json(std::istream& in, Nested &v);

void to_json(std::ostream& out, const Compound &v);
void from_json(std::istream& in, Compound &v);

void to_json(std::ostream& out, const OptionalVectors &v);
void from_json(std::istream& in, OptionalVectors &v);

void to_json(std::ostream& out, const VectorTo &v);
void from_json(std::istream& in, VectorTo &v);


} // namespace sp

namespace std {

ostream &operator<<(ostream& out, const sp::Nested &v);
istream &operator>>(istream& in, sp::Nested &v);

ostream &operator<<(ostream& out, const sp::Compound &v);
istream &operator>>(istream& in, sp::Compound &v);

ostream &operator<<(ostream& out, const sp::OptionalVectors &v);
istream &operator>>(istream& in, sp::OptionalVectors &v);

ostream &operator<<(ostream& out, const sp::VectorTo &v);
istream &operator>>(istream& in, sp::VectorTo &v);

} // namespace std

namespace std {

template<>
struct hash<sp::Nested> {
    std::size_t operator()(const sp::Nested &v) const noexcept;
};

template<>
struct hash<sp::Compound> {
    std::size_t operator()(const sp::Compound &v) const noexcept;
};

template<>
struct hash<sp::OptionalVectors> {
    std::size_t operator()(const sp::OptionalVectors &v) const noexcept;
};

template<>
struct hash<sp::VectorTo> {
    std::size_t operator()(const sp::VectorTo &v) const noexcept;
};

} // namespace std

namespace std {

void swap(sp::Nested &a, sp::Nested &b) noexcept;

void swap(sp::Compound &a, sp::Compound &b) noexcept;

void swap(sp::OptionalVectors &a, sp::OptionalVectors &b) noexcept;

void swap(sp::VectorTo &a, sp::VectorTo &b) noexcept;

} // namespace std

