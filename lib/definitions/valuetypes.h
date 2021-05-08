#pragma once

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace valuetypes { 

struct TemplateParameter {
    std::string type {  } ;
    bool optional { false } ;
};

struct Member {
    std::string name {  } ;
    std::string type {  } ;
    std::optional<std::string> default_value {  } ;
    bool optional { false } ;
    std::optional<TemplateParameter> value_type {  } ;
    std::optional<std::vector<TemplateParameter>> value_types {  } ;
};

struct Definition {
    std::string name {  } ;
    std::vector<Member> members {  } ;
};

struct DefinitionStore {
    std::optional<std::string> ns {  } ;
    std::vector<Definition> types {  } ;
};

bool operator==(const TemplateParameter &a, const TemplateParameter &b) noexcept;
bool operator!=(const TemplateParameter &a, const TemplateParameter &b) noexcept;

bool operator==(const Member &a, const Member &b) noexcept;
bool operator!=(const Member &a, const Member &b) noexcept;

bool operator==(const Definition &a, const Definition &b) noexcept;
bool operator!=(const Definition &a, const Definition &b) noexcept;

bool operator==(const DefinitionStore &a, const DefinitionStore &b) noexcept;
bool operator!=(const DefinitionStore &a, const DefinitionStore &b) noexcept;


bool operator<(const TemplateParameter &a, const TemplateParameter &b) noexcept;
bool operator<=(const TemplateParameter &a, const TemplateParameter &b) noexcept;
bool operator>(const TemplateParameter &a, const TemplateParameter &b) noexcept;
bool operator>=(const TemplateParameter &a, const TemplateParameter &b) noexcept;

bool operator<(const Member &a, const Member &b) noexcept;
bool operator<=(const Member &a, const Member &b) noexcept;
bool operator>(const Member &a, const Member &b) noexcept;
bool operator>=(const Member &a, const Member &b) noexcept;

bool operator<(const Definition &a, const Definition &b) noexcept;
bool operator<=(const Definition &a, const Definition &b) noexcept;
bool operator>(const Definition &a, const Definition &b) noexcept;
bool operator>=(const Definition &a, const Definition &b) noexcept;

bool operator<(const DefinitionStore &a, const DefinitionStore &b) noexcept;
bool operator<=(const DefinitionStore &a, const DefinitionStore &b) noexcept;
bool operator>(const DefinitionStore &a, const DefinitionStore &b) noexcept;
bool operator>=(const DefinitionStore &a, const DefinitionStore &b) noexcept;


} // namespace valuetypes

namespace valuetypes { 

void to_json(std::ostream& out, const TemplateParameter &v);
void from_json(std::istream& in, TemplateParameter &v);

void to_json(std::ostream& out, const Member &v);
void from_json(std::istream& in, Member &v);

void to_json(std::ostream& out, const Definition &v);
void from_json(std::istream& in, Definition &v);

void to_json(std::ostream& out, const DefinitionStore &v);
void from_json(std::istream& in, DefinitionStore &v);


} // namespace valuetypes

namespace std {

ostream &operator<<(ostream& out, const valuetypes::TemplateParameter &v);
istream &operator>>(istream& in, valuetypes::TemplateParameter &v);

ostream &operator<<(ostream& out, const valuetypes::Member &v);
istream &operator>>(istream& in, valuetypes::Member &v);

ostream &operator<<(ostream& out, const valuetypes::Definition &v);
istream &operator>>(istream& in, valuetypes::Definition &v);

ostream &operator<<(ostream& out, const valuetypes::DefinitionStore &v);
istream &operator>>(istream& in, valuetypes::DefinitionStore &v);

} // namespace std

namespace std {

template<>
struct hash<valuetypes::TemplateParameter> {
    std::size_t operator()(const valuetypes::TemplateParameter &v) const noexcept;
};

template<>
struct hash<valuetypes::Member> {
    std::size_t operator()(const valuetypes::Member &v) const noexcept;
};

template<>
struct hash<valuetypes::Definition> {
    std::size_t operator()(const valuetypes::Definition &v) const noexcept;
};

template<>
struct hash<valuetypes::DefinitionStore> {
    std::size_t operator()(const valuetypes::DefinitionStore &v) const noexcept;
};

} // namespace std

