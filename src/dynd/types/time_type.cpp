//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/array.hpp>
#include <dynd/func/callable.hpp>
#include <dynd/types/time_type.hpp>
#include <dynd/types/property_type.hpp>
#include <dynd/types/typevar_type.hpp>
#include <dynd/parser_util.hpp>

using namespace std;
using namespace dynd;

ndt::time_type::time_type(datetime_tz_t timezone)
    : base_type(time_type_id, datetime_kind, 8, scalar_align_of<int64_t>::value, type_flag_none, 0, 0, 0),
      m_timezone(timezone)
{
}

ndt::time_type::~time_type() {}

void ndt::time_type::set_time(const char *DYND_UNUSED(arrmeta), char *data, assign_error_mode errmode, int32_t hour,
                              int32_t minute, int32_t second, int32_t tick) const
{
  if (errmode != assign_error_nocheck && !time_hmst::is_valid(hour, minute, second, tick)) {
    stringstream ss;
    ss << "invalid input time " << hour << ":" << minute << ":" << second << ", ticks: " << tick;
    throw runtime_error(ss.str());
  }

  *reinterpret_cast<int64_t *>(data) = time_hmst::to_ticks(hour, minute, second, tick);
}

void ndt::time_type::set_from_utf8_string(const char *DYND_UNUSED(arrmeta), char *data, const char *utf8_begin,
                                          const char *utf8_end, const eval::eval_context *DYND_UNUSED(ectx)) const
{
  time_hmst hmst;
  const char *tz_begin = NULL, *tz_end = NULL;
  // TODO: Use errmode to adjust strictness of the parsing
  hmst.set_from_str(utf8_begin, utf8_end, tz_begin, tz_end);
  if (m_timezone != tz_abstract && tz_begin != tz_end) {
    if (m_timezone == tz_utc && (parse::compare_range_to_literal(tz_begin, tz_end, "Z") ||
                                 parse::compare_range_to_literal(tz_begin, tz_end, "UTC"))) {
      // It's a UTC time to a UTC time zone
    }
    else {
      stringstream ss;
      ss << "DyND time zone support is partial, cannot handle ";
      ss.write(tz_begin, tz_end - tz_begin);
      throw runtime_error(ss.str());
    }
  }
  *reinterpret_cast<int64_t *>(data) = hmst.to_ticks();
}

time_hmst ndt::time_type::get_time(const char *DYND_UNUSED(arrmeta), const char *data) const
{
  time_hmst hmst;
  hmst.set_from_ticks(*reinterpret_cast<const int64_t *>(data));
  return hmst;
}

void ndt::time_type::print_data(std::ostream &o, const char *DYND_UNUSED(arrmeta), const char *data) const
{
  time_hmst hmst;
  hmst.set_from_ticks(*reinterpret_cast<const int64_t *>(data));
  o << hmst.to_str();
  if (m_timezone == tz_utc) {
    o << "Z";
  }
}

void ndt::time_type::print_type(std::ostream &o) const
{
  if (m_timezone == tz_abstract) {
    o << "time";
  }
  else {
    o << "time[tz='";
    switch (m_timezone) {
    case tz_utc:
      o << "UTC";
      break;
    default:
      o << "(invalid " << (int32_t)m_timezone << ")";
      break;
    }
    o << "']";
  }
}

bool ndt::time_type::is_lossless_assignment(const type &dst_tp, const type &src_tp) const
{
  if (dst_tp.extended() == this) {
    if (src_tp.extended() == this) {
      return true;
    }
    else if (src_tp.get_type_id() == time_type_id) {
      // There is only one possibility for the time type (TODO: timezones!)
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

bool ndt::time_type::operator==(const base_type &rhs) const
{
  if (this == &rhs) {
    return true;
  }
  else if (rhs.get_type_id() != time_type_id) {
    return false;
  }
  else {
    const time_type &r = static_cast<const time_type &>(rhs);
    // TODO: When "other" timezone data is supported, need to compare them too
    return m_timezone == r.m_timezone;
  }
}

size_t ndt::time_type::make_comparison_kernel(void *ckb, intptr_t ckb_offset, const type &src0_tp,
                                              const char *src0_arrmeta, const type &src1_tp, const char *src1_arrmeta,
                                              comparison_type_t comptype, const eval::eval_context *ectx) const
{
  if (this == src0_tp.extended()) {
    if (*this == *src1_tp.extended()) {
      return make_builtin_type_comparison_kernel(ckb, ckb_offset, int64_type_id, int64_type_id, comptype);
    }
    else if (!src1_tp.is_builtin()) {
      return src1_tp.extended()->make_comparison_kernel(ckb, ckb_offset, src0_tp, src0_arrmeta, src1_tp, src1_arrmeta,
                                                        comptype, ectx);
    }
  }

  throw not_comparable_error(src0_tp, src1_tp, comptype);
}

///////// properties on the nd::array

struct get_hour_kernel : nd::base_kernel<get_hour_kernel> {
  nd::array self;

  get_hour_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    get_hour_kernel::make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "hour"));
  }
};

struct get_minute_kernel : nd::base_kernel<get_minute_kernel> {
  nd::array self;

  get_minute_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    get_minute_kernel::make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "minute"));
  }
};

struct get_second_kernel : nd::base_kernel<get_second_kernel> {
  nd::array self;

  get_second_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    get_second_kernel::make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "second"));
  }
};

struct get_microsecond_kernel : nd::base_kernel<get_microsecond_kernel> {
  nd::array self;

  get_microsecond_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    get_microsecond_kernel::make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "microsecond"));
  }
};

struct get_tick_kernel : nd::base_kernel<get_tick_kernel> {
  nd::array self;

  get_tick_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    get_tick_kernel::make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "tick"));
  }
};

void ndt::time_type::get_dynamic_array_properties(std::map<std::string, nd::callable> &properties) const
{
  static const std::map<std::string, nd::callable> time_array_properties{
      {"hour", nd::callable::make<get_hour_kernel>(ndt::type("(self: Any) -> Any"))},
      {"minute", nd::callable::make<get_minute_kernel>(ndt::type("(self: Any) -> Any"))},
      {"second", nd::callable::make<get_second_kernel>(ndt::type("(self: Any) -> Any"))},
      {"microsecond", nd::callable::make<get_microsecond_kernel>(ndt::type("(self: Any) -> Any"))},
      {"tick", nd::callable::make<get_tick_kernel>(ndt::type("(self: Any) -> Any"))}};

  properties = time_array_properties;
}

///////// functions on the nd::array

struct to_struct_kernel : nd::base_kernel<to_struct_kernel> {
  nd::array self;

  to_struct_kernel(const nd::array &self) : self(self) {}

  void single(nd::array *dst, nd::array *const *DYND_UNUSED(src)) { *dst = helper(self); }

  static void resolve_dst_type(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), ndt::type &dst_tp,
                               intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                               intptr_t DYND_UNUSED(nkwd), const nd::array *kwds,
                               const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    dst_tp = helper(kwds[0]).get_type();
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              const eval::eval_context *DYND_UNUSED(ectx), intptr_t DYND_UNUSED(nkwd),
                              const nd::array *kwds, const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    make(ckb, kernreq, ckb_offset, kwds[0]);
    return ckb_offset;
  }

  static nd::array helper(const nd::array &n)
  {
    return n.replace_dtype(ndt::property_type::make(n.get_dtype(), "struct"));
  }
};

void ndt::time_type::get_dynamic_array_functions(std::map<std::string, nd::callable> &functions) const
{
  functions["to_struct"] = nd::callable::make<to_struct_kernel>(ndt::type("(self: Any) -> Any"));
}

///////// property accessor kernels (used by property_type)

namespace {

struct time_get_hour_kernel : nd::base_kernel<time_get_hour_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    int64_t ticks = **reinterpret_cast<int64_t *const *>(src);
    *reinterpret_cast<int32_t *>(dst) = static_cast<int32_t>(ticks / DYND_TICKS_PER_HOUR);
  }
};

struct time_get_minute_kernel : nd::base_kernel<time_get_minute_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    int64_t ticks = **reinterpret_cast<int64_t *const *>(src);
    *reinterpret_cast<int32_t *>(dst) = static_cast<int32_t>((ticks / DYND_TICKS_PER_MINUTE) % 60);
  }
};

struct time_get_second_kernel : nd::base_kernel<time_get_second_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    int64_t ticks = **reinterpret_cast<int64_t *const *>(src);
    *reinterpret_cast<int32_t *>(dst) = static_cast<int32_t>((ticks / DYND_TICKS_PER_SECOND) % 60);
  }
};

struct time_get_microsecond_kernel : nd::base_kernel<time_get_microsecond_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    int64_t ticks = **reinterpret_cast<int64_t *const *>(src);
    *reinterpret_cast<int32_t *>(dst) = static_cast<int32_t>((ticks / DYND_TICKS_PER_MICROSECOND) % 1000000);
  }
};

struct time_get_tick_kernel : nd::base_kernel<time_get_tick_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    int64_t ticks = **reinterpret_cast<int64_t *const *>(src);
    *reinterpret_cast<int32_t *>(dst) = static_cast<int32_t>(ticks % 10000000);
  }
};

struct time_get_struct_kernel : nd::base_kernel<time_get_struct_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    time_hmst *dst_struct = reinterpret_cast<time_hmst *>(dst);
    dst_struct->set_from_ticks(**reinterpret_cast<int64_t *const *>(src));
  }
};

struct time_set_struct_kernel : nd::base_kernel<time_set_struct_kernel, 1> {
  void single(char *dst, char *const *src)
  {
    time_hmst *src_struct = *reinterpret_cast<time_hmst *const *>(src);
    *reinterpret_cast<int64_t *>(dst) = src_struct->to_ticks();
  }
};

} // anonymous namespace

namespace {
enum time_properties_t {
  timeprop_hour,
  timeprop_minute,
  timeprop_second,
  timeprop_microsecond,
  timeprop_tick,
  timeprop_struct
};
}

size_t ndt::time_type::get_elwise_property_index(const std::string &property_name) const
{
  if (property_name == "hour") {
    return timeprop_hour;
  }
  else if (property_name == "minute") {
    return timeprop_minute;
  }
  else if (property_name == "second") {
    return timeprop_second;
  }
  else if (property_name == "microsecond") {
    return timeprop_microsecond;
  }
  else if (property_name == "tick") {
    return timeprop_tick;
  }
  else if (property_name == "struct") {
    // A read/write property for accessing a time as a struct
    return timeprop_struct;
  }
  else {
    stringstream ss;
    ss << "dynd time type does not have a kernel for property " << property_name;
    throw runtime_error(ss.str());
  }
}

ndt::type ndt::time_type::get_elwise_property_type(size_t property_index, bool &out_readable, bool &out_writable) const
{
  switch (property_index) {
  case timeprop_hour:
  case timeprop_minute:
  case timeprop_second:
  case timeprop_microsecond:
  case timeprop_tick:
    out_readable = true;
    out_writable = false;
    return type::make<int32_t>();
  case timeprop_struct:
    out_readable = true;
    out_writable = true;
    return time_hmst::type();
  default:
    out_readable = false;
    out_writable = false;
    return type::make<void>();
  }
}

size_t ndt::time_type::make_elwise_property_getter_kernel(void *ckb, intptr_t ckb_offset,
                                                          const char *DYND_UNUSED(dst_arrmeta),
                                                          const char *DYND_UNUSED(src_arrmeta),
                                                          size_t src_property_index, kernel_request_t kernreq,
                                                          const eval::eval_context *DYND_UNUSED(ectx)) const
{
  switch (src_property_index) {
  case timeprop_hour:
    time_get_hour_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  case timeprop_minute:
    time_get_minute_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  case timeprop_second:
    time_get_second_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  case timeprop_microsecond:
    time_get_microsecond_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  case timeprop_tick:
    time_get_tick_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  case timeprop_struct:
    time_get_struct_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  default:
    stringstream ss;
    ss << "dynd time type given an invalid property index" << src_property_index;
    throw runtime_error(ss.str());
  }
}

size_t ndt::time_type::make_elwise_property_setter_kernel(
    void *ckb, intptr_t ckb_offset, const char *DYND_UNUSED(dst_arrmeta), size_t dst_property_index,
    const char *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq, const eval::eval_context *DYND_UNUSED(ectx)) const
{
  switch (dst_property_index) {
  case timeprop_struct:
    time_set_struct_kernel::make(ckb, kernreq, ckb_offset);
    return ckb_offset;
  default:
    stringstream ss;
    ss << "dynd time type given an invalid property index" << dst_property_index;
    throw runtime_error(ss.str());
  }
}
