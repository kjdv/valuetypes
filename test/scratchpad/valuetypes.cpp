#include "valuetypes.h"
#include <algorithm>
#include <cassert>
#include <composite/make.hh>
#include <kjson/builder.hh>
#include <kjson/json.hh>
#include <kjson/visitor.hh>
#include <optional>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace vt { 

bool operator==(const Nested &a, const Nested &b) noexcept {
    return
        std::tie(a.s) ==
        std::tie(b.s);
}

bool operator!=(const Nested &a, const Nested &b) noexcept {
    return !(a == b);
}

bool operator==(const Compound &a, const Compound &b) noexcept {
    return
        std::tie(a.a, a.b) ==
        std::tie(b.a, b.b);
}

bool operator!=(const Compound &a, const Compound &b) noexcept {
    return !(a == b);
}


bool operator<(const Nested &a, const Nested &b) noexcept {
    return
        std::tie(a.s) <
        std::tie(b.s);
}

bool operator<=(const Nested &a, const Nested &b) noexcept {
    return !(b < a);
}

bool operator>(const Nested &a, const Nested &b) noexcept {
    return b < a;
}

bool operator>=(const Nested &a, const Nested &b) noexcept {
    return !(a < b);
}

bool operator<(const Compound &a, const Compound &b) noexcept {
    return
        std::tie(a.a, a.b) <
        std::tie(b.a, b.b);
}

bool operator<=(const Compound &a, const Compound &b) noexcept {
    return !(b < a);
}

bool operator>(const Compound &a, const Compound &b) noexcept {
    return b < a;
}

bool operator>=(const Compound &a, const Compound &b) noexcept {
    return !(a < b);
}



} // } // namespace vt

// start iostream_definitions.cpp.inja

namespace vt { 
namespace {

template <typename T>
struct is_optional : std::false_type
{};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type
{};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

template <typename T>
struct is_vector : std::false_type
{};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type
{};

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

void to_kjson(kjson::builder &builder, const Nested &v);
void from_kjson(const kjson::document &doc, Nested &target);

void to_kjson(kjson::builder &builder, const Compound &v);
void from_kjson(const kjson::document &doc, Compound &target);

template <typename To>
To extract(kjson::scalar_t v) {
    return std::visit([](auto value) -> To {
        if constexpr(std::is_convertible_v<decltype(value), To>) {
            return value;
        } else {
            throw std::runtime_error("cannot convert");
        }
    },
                      v);
}

template <typename T>
void assign(T& target, kjson::scalar_t v) {
    target = extract<T>(std::move(v));
}

void check(kjson::maybe_error me) {
    if(me.is_err()) {
        throw std::runtime_error(me.unwrap_err().msg);
    }
}

class StackedVisitor : public kjson::visitor {
  public:
    void set_delegate(std::unique_ptr<StackedVisitor> d) {
        d_delegate = std::move(d);
    }
    void pop() final {
        if(d_delegate) {
            d_delegate->pop();

            if(!d_delegate->has_delegate()) {
                d_delegate.reset();
            }
        }
    }

    bool has_delegate() const {
        return d_delegate.get();
    }

    StackedVisitor& delegate() {
        assert(d_delegate);
        return *d_delegate;
    }

  private:
    std::unique_ptr<StackedVisitor> d_delegate;
};

class PopulateState {
  public:
    virtual ~PopulateState() = default;

    virtual void scalar(std::string_view key, kjson::scalar_t v) = 0;

    virtual std::unique_ptr<PopulateState> push_mapping(std::string_view key) = 0;
};

class PopulateNestedState : public PopulateState {
  public:
    PopulateNestedState(Nested& target)
      : d_target(target) {}

    void scalar(std::string_view key, kjson::scalar_t v) override {
        if(key == "s") {
            assign(d_target.s, std::move(v));
        }
    }

    std::unique_ptr<PopulateState> push_mapping(std::string_view key) override {
        assert(false && "not implemented");
        return nullptr;
    }

  private:
    Nested& d_target;
};

class PopulateCompoundState : public PopulateState {
  public:
    PopulateCompoundState(Compound& target)
      : d_target(target) {}

    void scalar(std::string_view key, kjson::scalar_t v) override {
        assert(false);
    }
    std::unique_ptr<PopulateState> push_mapping(std::string_view key) override {
        if(key == "a") {
            return std::make_unique<PopulateNestedState>(d_target.a);
        } else if(key == "b") {
            return std::make_unique<PopulateNestedState>(d_target.b);
        } else {
            return nullptr;
        }
    }

  private:
    Compound& d_target;
};

class PopulateStateMachine : public kjson::visitor {
  public:
    PopulateStateMachine(std::unique_ptr<PopulateState> initial) {
        d_states.push(std::move(initial));
    }

    void scalar(kjson::scalar_t v) override {
        //        state().scalar(std::move(v));
    }
    void scalar(std::string_view key, kjson::scalar_t v) override {
        state().scalar(key, std::move(v));
    }

    void push_sequence() override {
        // state().push_sequence();
    }

    void push_sequence(std::string_view key) override {
        // state().push_sequence(key);
    }

    void push_mapping() override {
        // state().push_mapping();
    }
    void push_mapping(std::string_view key) override {
        auto next = state().push_mapping(key);
        d_states.push(std::move(next));
    }

    void pop() override {
        d_states.pop();
    }

  private:
    PopulateState& state() {
        assert(!d_states.empty());
        return *d_states.top();
    }

    std::stack<std::unique_ptr<PopulateState>> d_states;
};

template <typename T>
void to_kjson(kjson::builder& builder, const T& v) {
    if constexpr (is_optional_v<T>) {
        if (!v) {
            builder.with_none();
        } else {
            to_kjson(builder, *v);
        }
    } else if constexpr (is_vector_v<T>) {
        builder.push_sequence();
        for (auto&& item : v) {
            to_kjson(builder, item);
        }
        builder.pop();
    } else {
        builder.value(v);
    }
}

void to_kjson(kjson::builder &builder, const Nested &v) {
    builder.push_mapping();
    builder.key("s");
    to_kjson(builder, v.s);
    builder.pop();
}

void to_kjson(kjson::builder &builder, const Compound &v) {
    builder.push_mapping();
    builder.key("a");
    to_kjson(builder, v.a);
    builder.key("b");
    to_kjson(builder, v.b);
    builder.pop();
}


} // anonymous namespace

void to_json(std::ostream& out, const Nested &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Nested &v) {
    PopulateStateMachine visitor(std::make_unique<PopulateNestedState>(v));
    check(kjson::load(in, visitor));
}

void to_json(std::ostream& out, const Compound &v) {
    kjson::builder builder(out, true);
    to_kjson(builder, v);
}

void from_json(std::istream& in, Compound &v) {
    PopulateStateMachine visitor(std::make_unique<PopulateCompoundState>(v));
    check(kjson::load(in, visitor));
}

} // namespace vt

namespace std {

std::ostream &operator<<(std::ostream& out, const vt::Nested &v) {
    vt::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, vt::Nested &v) {
    vt::from_json(in, v);
    return in;
}

std::ostream &operator<<(std::ostream& out, const vt::Compound &v) {
    vt::to_json(out, v);
    return out;
}

std::istream &operator>>(std::istream& in, vt::Compound &v) {
    vt::from_json(in, v);
    return in;
}

} // namespace std

// end iostream_definitions.cpp.inja

// start hash_definitions.cpp.inja

namespace std {

namespace {

constexpr std::size_t combine(std::size_t a, std::size_t b) noexcept {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

template <typename T>
constexpr std::size_t base_hash(const T&v) noexcept {
    std::hash<T> hasher;
    return hasher(v);
}

template <typename T>
constexpr std::size_t base_hash(const std::vector<T> &v) noexcept {
    std::size_t h{0};
    std::hash<T> ih;
    for (auto&& item : v) {
        h = combine(h, ih(item));
    }
    return h;
}

template <typename T>
constexpr std::size_t base_hash(const std::optional<T> &v) noexcept {
    return v ? base_hash(*v) : 0;
}

std::size_t hash_combine() {
    return 0;
}

template <typename Head, typename... Tail>
std::size_t hash_combine(const Head &head, Tail... tail) {
    auto h = base_hash(head);
    auto t = hash_combine(std::forward<Tail>(tail)...);
    return combine(t, h);
}

} // anonymous namespace

std::size_t hash<vt::Nested>::operator()(const vt::Nested &v) const noexcept {
    return hash_combine(v.s);
}

std::size_t hash<vt::Compound>::operator()(const vt::Compound &v) const noexcept {
    return hash_combine(v.a, v.b);
}

} // namespace std

// end hash_definitions.cpp.inja
// start swap_definitions.cpp.inja

namespace std {

void swap(vt::Nested &a, vt::Nested &b) noexcept {
    swap(a.s, b.s);
}

void swap(vt::Compound &a, vt::Compound &b) noexcept {
    swap(a.a, b.a);
    swap(a.b, b.b);
}

} // namespace std

// end swap_definitions.cpp.inja
