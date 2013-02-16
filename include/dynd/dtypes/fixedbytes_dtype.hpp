//
// Copyright (C) 2011-13, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef _DYND__FIXEDBYTES_DTYPE_HPP_
#define _DYND__FIXEDBYTES_DTYPE_HPP_

#include <dynd/dtype.hpp>
#include <dynd/dtype_assign.hpp>
#include <dynd/dtypes/base_bytes_dtype.hpp>
#include <dynd/dtypes/view_dtype.hpp>

namespace dynd {

class fixedbytes_dtype : public base_bytes_dtype {
public:
    fixedbytes_dtype(intptr_t element_size, intptr_t alignment);

    virtual ~fixedbytes_dtype();

    void print_data(std::ostream& o, const char *metadata, const char *data) const;

    void print_dtype(std::ostream& o) const;

    // This is about the native storage, buffering code needs to check whether
    // the value_dtype is an object type separately.
    dtype_memory_management_t get_memory_management() const {
        return pod_memory_management;
    }

    void get_bytes_range(const char **out_begin, const char**out_end, const char *metadata, const char *data) const;

    bool is_lossless_assignment(const dtype& dst_dt, const dtype& src_dt) const;

    void get_single_compare_kernel(kernel_instance<compare_operations_t>& out_kernel) const;

    bool operator==(const base_dtype& rhs) const;

    void metadata_default_construct(char *DYND_UNUSED(metadata), size_t DYND_UNUSED(ndim), const intptr_t* DYND_UNUSED(shape)) const {
    }
    void metadata_copy_construct(char *DYND_UNUSED(dst_metadata), const char *DYND_UNUSED(src_metadata), memory_block_data *DYND_UNUSED(embedded_reference)) const {
    }
    void metadata_destruct(char *DYND_UNUSED(metadata)) const {
    }
    void metadata_debug_print(const char *DYND_UNUSED(metadata), std::ostream& DYND_UNUSED(o), const std::string& DYND_UNUSED(indent)) const {
    }

    size_t make_assignment_kernel(
                    assignment_kernel *out, size_t offset_out,
                    const dtype& dst_dt, const char *dst_metadata,
                    const dtype& src_dt, const char *src_metadata,
                    assign_error_mode errmode,
                    const eval::eval_context *ectx) const;
};

/**
 * Creates a bytes<size, alignment> dtype, for representing
 * raw, uninterpreted bytes.
 */
inline dtype make_fixedbytes_dtype(intptr_t element_size, intptr_t alignment) {
    return dtype(new fixedbytes_dtype(element_size, alignment), false);
}

} // namespace dynd

#endif // _DYND__FIXEDBYTES_DTYPE_HPP_
