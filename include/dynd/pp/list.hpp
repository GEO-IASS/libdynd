//
// Copyright (C) 2011-14 Irwin Zaid, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef _DYND__PP_LIST_HPP_
#define _DYND__PP_LIST_HPP_

#include <dynd/pp/arithmetic.hpp>
#include <dynd/pp/comparison.hpp>
#include <dynd/pp/gen.hpp>
#include <dynd/pp/if.hpp>
#include <dynd/pp/logical.hpp>
#include <dynd/pp/token.hpp>

/**
 * Expands to A without its enclosing parentheses.
 */
#define DYND_PP_FLATTEN(A) DYND_PP_ID A

#define DYND_PP_LIFT(...) (__VA_ARGS__)

/**
 * Expands to 1 if A is a pair of parentheses enclosing whitespace. Otherwise 0.
 */
// #define DYND_PP_IS_EMPTY(A) DYND_PP_BOOL(DYND_PP_LEN(A))
#define DYND_PP_IS_EMPTY(A) DYND_PP_IS_NULL A

/**
 * Expands to the first token in A if A is not empty. Otherwise expands to whitespace.
 */
#define DYND_PP_FIRST(A) DYND_PP_PASTE(DYND_PP_FIRST_IF_, DYND_PP_IF_ELSE(DYND_PP_IS_EMPTY(A))(EMPTY)(NOT_EMPTY))(A)
#define DYND_PP_FIRST_IF_EMPTY(A)
#define DYND_PP_FIRST_IF_NOT_EMPTY(A) DYND_PP__FIRST_IF_NOT_EMPTY(DYND_PP_ID A)
#define DYND_PP__FIRST_IF_NOT_EMPTY(...) DYND_PP_ID(DYND_PP___FIRST_IF_NOT_EMPTY(__VA_ARGS__))
#define DYND_PP___FIRST_IF_NOT_EMPTY(A0, ...) A0

#define DYND_PP_POP(A) DYND_PP_DEL(DYND_PP_DEC(DYND_PP_LEN(A)), A)

/**
 * Expands to A without its first token if A is not empty. Otherwise expands to ().
 */
#define DYND_PP_POP_FIRST(A) DYND_PP_PASTE(DYND_PP_POP_FIRST_IF_, DYND_PP_IF_ELSE(DYND_PP_IS_EMPTY(A))(EMPTY)(NOT_EMPTY))(A)
#define DYND_PP_POP_FIRST_IF_EMPTY(A) ()
#define DYND_PP_POP_FIRST_IF_NOT_EMPTY(A) DYND_PP__POP_FIRST_IF_NOT_EMPTY(DYND_PP_ID A)
#define DYND_PP__POP_FIRST_IF_NOT_EMPTY(...) DYND_PP_ID(DYND_PP___POP_FIRST_IF_NOT_EMPTY(__VA_ARGS__))
#define DYND_PP___POP_FIRST_IF_NOT_EMPTY(A0, ...) (__VA_ARGS__)

#define DYND_PP_REST DYND_PP_POP_FIRST

#define DYND_PP_CHAIN DYND_PP_MERGE
#define DYND_PP_MERGE(...) DYND_PP_PASTE(DYND_PP_MERGE_, DYND_PP_LEN((__VA_ARGS__)))(__VA_ARGS__)
#define DYND_PP_MERGE_2(A, B) DYND_PP_IF_ELSE(DYND_PP_LEN(A))(DYND_PP_IF_ELSE(DYND_PP_LEN(B))((DYND_PP_ID A, \
    DYND_PP_ID B))(A))(DYND_PP_IF_ELSE(DYND_PP_LEN(B))(B)(()))
#define DYND_PP_MERGE_3(A, B, C) DYND_PP_MERGE_2(A, DYND_PP_MERGE_2(B, C))

#define DYND_PP_PREPEND(ITEM, A) DYND_PP_MERGE((ITEM), A)
#define DYND_PP_APPEND(ITEM, A) DYND_PP_MERGE(A, (ITEM))

//#define DYND_PP_SHIFT(A) DYND_PP_APPEND(DYND_PP_FIRST(A), DYND_PP_POP_FIRST(A))

#define DYND_PP_GET(INDEX, A) DYND_PP_FIRST(DYND_PP_SLICE_FROM(INDEX, A))

#define DYND_PP_SET(INDEX, VALUE, A) DYND_PP_MERGE(DYND_PP_APPEND(VALUE, DYND_PP_SLICE_TO(INDEX, A)), \
    DYND_PP_SLICE_FROM(DYND_PP_INC(INDEX), A))

#define DYND_PP_DEL(INDEX, A) DYND_PP_MERGE(DYND_PP_SLICE_TO(INDEX, A), \
    DYND_PP_SLICE_FROM(DYND_PP_INC(INDEX), A))

#define DYND_PP_LAST(A) DYND_PP_GET(DYND_PP_DEC(DYND_PP_LEN(A)), A)

#define DYND_PP_SLICE(...) DYND_PP_ID(DYND_PP__SLICE(__VA_ARGS__))
#define DYND_PP__SLICE(...) DYND_PP_ID(DYND_PP_PASTE(DYND_PP__SLICE_, DYND_PP_DEC(DYND_PP_LEN((__VA_ARGS__))))(__VA_ARGS__))
#define DYND_PP__SLICE_1(STOP, A) DYND_PP_SLICE_TO(STOP, A)
#define DYND_PP__SLICE_2(START, STOP, A) DYND_PP_SLICE_TO(DYND_PP_SUB(STOP, START), \
    DYND_PP_SLICE_FROM(START, A))
#define DYND_PP__SLICE_3(START, STOP, STEP, A) DYND_PP_SLICE_WITH(STEP, \
    DYND_PP_SLICE_TO(DYND_PP_SUB(STOP, START), DYND_PP_SLICE_FROM(START, A)))

#define DYND_PP_RANGE(...) DYND_PP_ID(DYND_PP__RANGE(__VA_ARGS__))
#define DYND_PP__RANGE(...) DYND_PP_SLICE(__VA_ARGS__, DYND_PP_INTS)

//#define DYND_PP_RANGE(...) DYND_PP_ID(DYND_PP_PASTE(DYND_PP_RANGE_, DYND_PP_LEN((__VA_ARGS__)))(__VA_ARGS__))
//#define DYND_PP_RANGE_1(STOP) DYND_PP_RANGE_2(0, STOP)
//#define DYND_PP_RANGE_2(START, STOP) DYND_PP_RANGE_3(START, STOP, 1)
//#define DYND_PP_RANGE_3(START, STOP, STEP) DYND_PP_SLICE(START, STOP, STEP, DYND_PP_INTS)

#define DYND_PP_ALL(A) DYND_PP_IF_ELSE(DYND_PP_EQ(DYND_PP_LEN(A), 1))( \
    DYND_PP_BOOL(DYND_PP_FIRST(A)))(DYND_PP_REDUCE(DYND_PP_AND, A))

#define DYND_PP_ANY(A) DYND_PP_IF_ELSE(DYND_PP_EQ(DYND_PP_LEN(A), 1))( \
    DYND_PP_BOOL(DYND_PP_FIRST(A)))(DYND_PP_REDUCE(DYND_PP_OR, A))

#define DYND_PP_ALL_LT(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_LT, A, B))
#define DYND_PP_ANY_LT(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_LT, A, B))

#define DYND_PP_ALL_LE(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_LE, A, B))
#define DYND_PP_ANY_LE(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_LE, A, B))

#define DYND_PP_ALL_EQ(A, B) DYND_PP_IF_ELSE(DYND_PP_EQ(DYND_PP_LEN(A), \
    DYND_PP_LEN(B)))(DYND_PP_ALL_EQ_SAME_LEN)(DYND_PP_ALL_EQ_DIFF_LEN)(A, B)
#define DYND_PP_ALL_EQ_SAME_LEN(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_EQ, A, B))
#define DYND_PP_ALL_EQ_DIFF_LEN(A, B) 0

#define DYND_PP_ANY_EQ(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_EQ, A, B))

#define DYND_PP_ALL_NE(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_NE, A, B))
#define DYND_PP_ANY_NE(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_NE, A, B))

#define DYND_PP_ALL_GE(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_GE, A, B))
#define DYND_PP_ANY_GE(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_GE, A, B))

#define DYND_PP_ALL_GT(A, B) DYND_PP_ALL(DYND_PP_ELWISE(DYND_PP_GT, A, B))
#define DYND_PP_ANY_GT(A, B) DYND_PP_ANY(DYND_PP_ELWISE(DYND_PP_GT, A, B))

#endif
