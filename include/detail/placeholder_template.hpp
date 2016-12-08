#ifndef PLACEHOLDER_TEMPLATE_HPP
#define PLACEHOLDER_TEMPLATE_HPP

#include <functional>
#include <type_traits>

namespace mcurses {

// Allows you to bind a variable number of std::placeholder arguments to a
// std::bind call. Used in Connection_impl::bind_connection().
template <int>
struct Placeholder_template {};

}  // namespace mcurses

namespace std {

template <int N>
struct is_placeholder<mcurses::Placeholder_template<N>>
    : public integral_constant<int, N + 1> {};

}  // namespace std

#endif  // PLACEHOLDER_TEMPLATE_HPP
