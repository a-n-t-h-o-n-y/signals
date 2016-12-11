/// \file
/// \brief Contains the forward declaration of the Slot class template.
///
/// Include this header instead of forward declaring the class yourself.
#ifndef SLOT_FWD_HPP
#define SLOT_FWD_HPP

#include <functional>

namespace mcurses {

template <typename Signature, typename FunctionType = std::function<Signature>>
class Slot;

}  // namespace mcurses

#endif  // SLOT_FWD_HPP
