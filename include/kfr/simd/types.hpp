/** @addtogroup types
 *  @{
 */
/*
  Copyright (C) 2016 D Levin (https://www.kfrlib.com)
  This file is part of KFR

  KFR is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  KFR is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KFR.

  If GPL is not suitable for your project, you must purchase a commercial license to use KFR.
  Buying a commercial license is mandatory as soon as you develop commercial activities without
  disclosing the source code of your own applications.
  See https://www.kfrlib.com for details.
 */
#pragma once

#include "../kfr.h"

#include "impl/intrinsics.h"

#include <climits>

#include <cmath>
#include <limits>
#include <random>

CMT_PRAGMA_GNU(GCC diagnostic push)
CMT_PRAGMA_GNU(GCC diagnostic ignored "-Wshadow")
CMT_PRAGMA_GNU(GCC diagnostic ignored "-Wignored-qualifiers")

#ifdef KFR_TESTING
#include "../testo/testo.hpp"
#include "../cometa/function.hpp"
#endif

#include "../cometa.hpp"
#include "../cometa/numeric.hpp"

namespace kfr
{

// Include all from CoMeta library
using namespace cometa;

using cometa::fbase;
using cometa::fmax;

// primary template (used for zero types)
template <typename... T>
struct common_type_impl
{
};

template <typename... T>
using decay_common = decay<common_type_impl<T...>>;

template <typename T1, typename T2, template <typename TT> class result_type, typename = void>
struct common_type_from_subtypes
{
};

template <typename T1, typename T2, template <typename TT> class result_type>
struct common_type_from_subtypes<T1, T2, result_type, void_t<typename common_type_impl<T1, T2>::type>>
{
    using type = result_type<typename common_type_impl<T1, T2>::type>;
};

template <typename T>
struct common_type_impl<T>
{
    using type = decay<T>;
};

template <typename T1, typename T2>
using common_for_two = decltype(false ? std::declval<T1>() : std::declval<T2>());

template <typename T1, typename T2, typename = void>
struct common_type_2_default
{
};

template <typename T1, typename T2>
struct common_type_2_default<T1, T2, void_t<common_for_two<T1, T2>>>
{
    using type = std::decay_t<common_for_two<T1, T2>>;
};

template <typename T1, typename T2, typename D1 = decay<T1>, typename D2 = decay<T2>>
struct common_type_2_impl : common_type_impl<D1, D2>
{
};

template <typename D1, typename D2>
struct common_type_2_impl<D1, D2, D1, D2> : common_type_2_default<D1, D2>
{
};

template <typename T1, typename T2>
struct common_type_impl<T1, T2> : common_type_2_impl<T1, T2>
{
};

template <typename AlwaysVoid, typename T1, typename T2, typename... R>
struct common_type_multi_impl
{
};

template <typename T1, typename T2, typename... R>
struct common_type_multi_impl<void_t<typename common_type_impl<T1, T2>::type>, T1, T2, R...>
    : common_type_impl<typename common_type_impl<T1, T2>::type, R...>
{
};

template <typename T1, typename T2, typename... R>
struct common_type_impl<T1, T2, R...> : common_type_multi_impl<void, T1, T2, R...>
{
};

template <typename... T>
using common_type = typename common_type_impl<T...>::type;

constexpr ctypes_t<i8, i16, i32, i64> signed_types{};
constexpr ctypes_t<u8, u16, u32, u64> unsigned_types{};
constexpr ctypes_t<i8, i16, i32, i64, u8, u16, u32, u64> integer_types{};
constexpr ctypes_t<f32
#ifdef KFR_NATIVE_F64
                   ,
                   f64
#endif
                   >
    float_types{};
constexpr ctypes_t<i8, i16, i32, i64, u8, u16, u32, u64, f32
#ifdef KFR_NATIVE_F64
                   ,
                   f64
#endif
                   >
    numeric_types{};

constexpr csizes_t<1, 2, 3, 4, 8, 16, 32, 64> test_vector_sizes{};

#ifdef CMT_ARCH_AVX512
constexpr size_t max_test_size = 128;
#elif defined CMT_ARCH_AVX
constexpr size_t max_test_size = 64;
#else
constexpr size_t max_test_size = 32;
#endif

template <template <typename, size_t> class vec_tpl, typename T,
          typename sizes =
#ifdef KFR_EXTENDED_TESTS
              cfilter_t<decltype(test_vector_sizes), decltype(test_vector_sizes <= csize<max_test_size / sizeof(T)>)>
#else
              csizes_t<1, 2>
#endif
          >
struct vector_types_for_size_t_impl;

template <template <typename, size_t> class vec_tpl, typename T, size_t... sizes>
struct vector_types_for_size_t_impl<vec_tpl, T, csizes_t<sizes...>>
{
    using type = ctypes_t<vec_tpl<T, sizes>...>;
};

template <template <typename, size_t> class vec_tpl, typename T>
using vector_types_for_size_t = typename vector_types_for_size_t_impl<vec_tpl, T>::type;

template <template <typename, size_t> class vec_tpl>
using signed_vector_types_t =
    concat_lists<vector_types_for_size_t<vec_tpl, i8>, vector_types_for_size_t<vec_tpl, i16>,
                 vector_types_for_size_t<vec_tpl, i32>, vector_types_for_size_t<vec_tpl, i64>>;

template <template <typename, size_t> class vec_tpl>
constexpr signed_vector_types_t<vec_tpl> signed_vector_types{};

template <template <typename, size_t> class vec_tpl>
using unsigned_vector_types_t =
    concat_lists<vector_types_for_size_t<vec_tpl, u8>, vector_types_for_size_t<vec_tpl, u16>,
                 vector_types_for_size_t<vec_tpl, u32>, vector_types_for_size_t<vec_tpl, u64>>;

template <template <typename, size_t> class vec_tpl>
constexpr unsigned_vector_types_t<vec_tpl> unsigned_vector_types{};

template <template <typename, size_t> class vec_tpl>
using integer_vector_types_t = concat_lists<signed_vector_types_t<vec_tpl>, unsigned_vector_types_t<vec_tpl>>;

template <template <typename, size_t> class vec_tpl>
constexpr integer_vector_types_t<vec_tpl> integer_vector_types{};

template <template <typename, size_t> class vec_tpl>
using float_vector_types_t = concat_lists<vector_types_for_size_t<vec_tpl, f32>
#ifdef KFR_NATIVE_F64
                                          ,
                                          vector_types_for_size_t<vec_tpl, f64>
#endif
                                          >;

template <template <typename, size_t> class vec_tpl>
constexpr float_vector_types_t<vec_tpl> float_vector_types{};

template <template <typename, size_t> class vec_tpl>
constexpr concat_lists<integer_vector_types_t<vec_tpl>, float_vector_types_t<vec_tpl>> numeric_vector_types{};

struct u24
{
    u8 raw[3];
};

struct i24
{
    u8 raw[3];

    constexpr i24(i32 x) CMT_NOEXCEPT : raw{}
    {
        raw[0] = x & 0xFF;
        raw[1] = (x >> 8) & 0xFF;
        raw[2] = (x >> 16) & 0xFF;
    }

    constexpr i32 as_int() const CMT_NOEXCEPT
    {
        return static_cast<i32>(raw[0]) | static_cast<i32>(raw[1] << 8) |
               (static_cast<i32>(raw[2] << 24) >> 8);
    }

    operator int() const CMT_NOEXCEPT { return as_int(); }
};

struct f16
{
    u16 raw;
};

template <size_t bits>
struct bitmask
{
    using type = conditional<(bits > 32), uint64_t,
                             conditional<(bits > 16), uint32_t, conditional<(bits > 8), uint16_t, uint8_t>>>;

    bitmask(type val) : value(val) {}

    type value;
};

template <typename T>
struct maskbit
{
    bool value;
};

namespace fn_generic
{
///@copybrief cometa::pass_through
using pass_through = cometa::fn_pass_through;

///@copybrief cometa::noop
using noop = cometa::fn_noop;

///@copybrief cometa::get_first
using get_first = cometa::fn_get_first;

///@copybrief cometa::get_second
using get_second = cometa::fn_get_second;

///@copybrief cometa::get_third
using get_third = cometa::fn_get_third;

///@copybrief cometa::returns
template <typename T>
using returns = cometa::fn_returns<T>;
} // namespace fn_generic

CMT_PRAGMA_GNU(GCC diagnostic push)
CMT_PRAGMA_GNU(GCC diagnostic ignored "-Wattributes")

template <typename T, bool A>
struct struct_with_alignment
{
    using pointer       = struct_with_alignment*;
    using const_pointer = const struct_with_alignment*;
    T value;
    KFR_MEM_INTRINSIC void operator=(T value) { this->value = value; }
};

#ifdef CMT_COMPILER_MSVC
#define KFR_UNALIGNED_POINTER __unaligned
#else
#define KFR_UNALIGNED_POINTER
#endif

template <typename T>
struct struct_with_alignment<T, false>
{
    using pointer       = KFR_UNALIGNED_POINTER struct_with_alignment*;
    using const_pointer = KFR_UNALIGNED_POINTER const struct_with_alignment*;
    T value;
    KFR_MEM_INTRINSIC void operator=(T value) { this->value = value; }
}
#ifdef CMT_GNU_ATTRIBUTES
__attribute__((__packed__, __may_alias__)) //
#endif
;

CMT_PRAGMA_GNU(GCC diagnostic pop)

/// @brief Fills a value with zeros
template <typename T1>
KFR_INTRINSIC void zeroize(T1& value)
{
    builtin_memset(static_cast<void*>(builtin_addressof(value)), 0, sizeof(T1));
}

/// @brief Used to determine the initial value for reduce functions
template <typename T>
struct initialvalue
{
};

template <typename T>
struct is_simd_type
    : std::integral_constant<
          bool, std::is_same<T, float>::value || std::is_same<T, double>::value ||
                    std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value ||
                    std::is_same<T, short>::value || std::is_same<T, unsigned short>::value ||
                    std::is_same<T, int>::value || std::is_same<T, unsigned int>::value ||
                    std::is_same<T, long>::value || std::is_same<T, unsigned long>::value ||
                    std::is_same<T, long long>::value || std::is_same<T, unsigned long long>::value>
{
};

template <typename T, size_t N>
struct vec_shape
{
    using value_type = T;
    constexpr static size_t size() CMT_NOEXCEPT { return N; }
    constexpr vec_shape() CMT_NOEXCEPT = default;

    using scalar_type = subtype<T>;
    constexpr static size_t scalar_size() CMT_NOEXCEPT { return N * compound_type_traits<T>::width; }
};

constexpr size_t index_undefined = static_cast<size_t>(-1);

struct czeros_t
{
};
struct cones_t
{
};
constexpr czeros_t czeros{};
constexpr cones_t cones{};

using caligned_t   = cbool_t<true>;
using cunaligned_t = cbool_t<false>;

constexpr caligned_t caligned{};
constexpr cunaligned_t cunaligned{};

#ifdef CMT_INTRINSICS_IS_CONSTEXPR
#define KFR_I_CE constexpr
#else
#define KFR_I_CE
#endif

#define avoid_odr_use(x) static_cast<decltype(x)>(x)

} // namespace kfr

CMT_PRAGMA_GNU(GCC diagnostic pop)