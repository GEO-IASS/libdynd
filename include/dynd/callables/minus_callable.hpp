//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/callables/apply_function_callable.hpp>
#include <dynd/kernels/arithmetic.hpp>

namespace dynd {
namespace nd {

  template <type_id_t Arg0ID>
  using minus_callable = functional::apply_function_callable<decltype(&dynd::detail::inline_minus<Arg0ID>::f),
                                                             &dynd::detail::inline_minus<Arg0ID>::f>;

} // namespace dynd::nd
} // namespace dynd
