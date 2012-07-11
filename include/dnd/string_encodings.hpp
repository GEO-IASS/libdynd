//
// Copyright (C) 2011-12, Dynamic NDArray Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef _DND__STRING_ENCODINGS_HPP_
#define _DND__STRING_ENCODINGS_HPP_

#include <iostream>

#include <dnd/config.hpp>
#include <dnd/dtype_assign.hpp>

namespace dnd {

enum string_encoding_t {
    string_encoding_ascii,
    string_encoding_ucs_2,
    string_encoding_utf_8,
    string_encoding_utf_16,
    string_encoding_utf_32,

    string_encoding_invalid
};

/**
 * A table of the individual character sizes for
 * the various encodings.
 */
extern int string_encoding_char_size_table[6];

inline std::ostream& operator<<(std::ostream& o, string_encoding_t encoding)
{
    switch (encoding) {
        case string_encoding_ascii:
            o << "ascii";
            break;
        case string_encoding_ucs_2:
            o << "ucs_2";
            break;
        case string_encoding_utf_8:
            o << "utf_8";
            break;
        case string_encoding_utf_16:
            o << "utf_16";
            break;
        case string_encoding_utf_32:
            o << "utf_32";
            break;
        default:
            o << "unknown string encoding";
            break;
    }

    return o;
}

/**
 * Typedef for getting the next unicode codepoint from a string of a particular
 * encoding.
 *
 * On entry, this function assumes that 'it' and 'end' are appropriately aligned
 * and that (it < end). The variable 'it' is updated in-place to be after the
 * character data representing the returned code point.
 *
 * This function may raise an exception if there is an error.
 */
typedef uint32_t (*next_unicode_codepoint_t)(const char *&it, const char *end);

/**
 * Typedef for appending a unicode codepoint to a string of a particular
 * encoding.
 *
 * On entry, this function assumes that 'it' and 'end' are appropriately aligned
 * and that (it < end). The variable 'it' is updated in-place to be after the
 * character data representing the appended code point.
 *
 * This function may raise an exception if there is an error.
 */
typedef void (*append_unicode_codepoint_t)(uint32_t cp, char *&it, char *end);

next_unicode_codepoint_t get_next_unicode_codepoint_function(string_encoding_t encoding, assign_error_mode errmode);
append_unicode_codepoint_t get_append_unicode_codepoint_function(string_encoding_t encoding, assign_error_mode errmode);

/**
 * Prints the given code point to the output stream, escaping it as necessary.
 */
void print_escaped_unicode_codepoint(std::ostream& o, uint32_t cp);

} // namespace dnd

#endif // _DND__STRING_ENCODINGS_HPP_
