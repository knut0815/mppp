// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(_MSC_VER)

// Disable some warnings for MSVC. These arise only in release mode apparently.
#pragma warning(push)
#pragma warning(disable : 4723)

#endif

#include <cstddef>
#include <gmp.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

template <typename T, typename U>
using divvv_t = decltype(std::declval<const T &>() / std::declval<const U &>());

template <typename T, typename U>
using inplace_divvv_t = decltype(std::declval<T &>() /= std::declval<const U &>());

template <typename T, typename U>
using is_divisible = is_detected<divvv_t, T, U>;

template <typename T, typename U>
using is_divisible_inplace = is_detected<inplace_divvv_t, T, U>;

struct div_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / char(4)) == "1"));
        REQUIRE((lex_cast(char(4) / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) / n2), integer>::value));
        REQUIRE((lex_cast(n1 / static_cast<unsigned char>(4)) == "1"));
        REQUIRE((lex_cast(static_cast<unsigned char>(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / short(4)) == "1"));
        REQUIRE((lex_cast(short(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / 4) == "1"));
        REQUIRE((lex_cast(4 / n2) == "-2"));
        REQUIRE((lex_cast(n1 / wchar_t{4}) == "1"));
        REQUIRE((lex_cast(wchar_t{4} / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / 4u) == "1"));
        REQUIRE((lex_cast(4u / n2) == "-2"));
        REQUIRE((n1 / 4.f == 1.f));
        REQUIRE((4.f / n2 == -2.f));
        REQUIRE((std::is_same<decltype(n1 / 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f / n2), float>::value));
        REQUIRE((n1 / 4. == 1.));
        REQUIRE((4. / n2 == -2.));
        REQUIRE((std::is_same<decltype(n1 / 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. / n2), double>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((n1 / 4.l == 1.l));
        REQUIRE((4.l / n2 == -2.l));
        REQUIRE((std::is_same<decltype(n1 / 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l / n2), long double>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 / __uint128_t{4} == 1));
        REQUIRE((__uint128_t{4} / n2 == -2));
        REQUIRE((n1 / __int128_t{-4} == -1));
        REQUIRE((__int128_t{-4} / n1 == -1));
#endif
        // In-place div.
        integer retval{2};
        retval /= n1;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 2;
        retval /= 1;
        REQUIRE((lex_cast(retval) == "2"));
        retval /= short(-1);
        REQUIRE((lex_cast(retval) == "-2"));
        retval /= static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval /= -5ll;
        REQUIRE((lex_cast(retval) == "0"));
        retval = -20;
        retval /= 20ull;
        REQUIRE((lex_cast(retval) == "-1"));
        retval /= 2.5f;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 10;
        retval /= -3.5;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5})));
#if defined(MPPP_WITH_MPFR)
        retval /= -1.5l;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5 / -1.5l})));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval /= __uint128_t{1};
        REQUIRE((retval == 1));
        retval /= __int128_t{-1};
        REQUIRE((retval == -1));
#endif
        // In-place with interop on the lhs.
        short nl = 12;
        nl /= integer{3};
        REQUIRE((std::is_same<short &, decltype(nl /= integer{1})>::value));
        REQUIRE(nl == 4);
        nl /= integer{-2};
        REQUIRE(nl == -2);
        REQUIRE_THROWS_AS(nl /= integer{}, zero_division_error);
        unsigned long long unl = 24;
        unl /= integer{2};
        REQUIRE(unl == 12);
        REQUIRE_THROWS_AS(unl /= integer{-1}, std::overflow_error);
        double dl = 1.2;
        dl /= integer{2};
        REQUIRE(dl == 1.2 / 2.);
        REQUIRE((std::is_same<double &, decltype(dl /= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl /= integer{2};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 /= integer{5};
        REQUIRE((n128 == -1));
        __uint128_t un128{6};
        un128 /= integer{3};
        REQUIRE((un128 == 2));
#endif
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} / 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE_THROWS_PREDICATE(integer{1} / __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} / __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
#endif
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((integer{4} / 0. == std::numeric_limits<double>::infinity()));
            REQUIRE((integer{-4} / 0. == -std::numeric_limits<double>::infinity()));
            REQUIRE_THROWS_PREDICATE(retval /= 0., std::domain_error, [&retval](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot assign the non-finite floating-point value "
                              + (retval.sgn() > 0 ? std::to_string(std::numeric_limits<double>::infinity())
                                                  : std::to_string(-std::numeric_limits<double>::infinity()))
                              + " to an integer";
            });
        }
        // Type traits.
        REQUIRE((!is_divisible<integer, std::string>::value));
        REQUIRE((!is_divisible<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<integer, std::string>::value));
        REQUIRE((!is_divisible_inplace<const integer, int>::value));
        REQUIRE((!is_divisible_inplace<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<const int, integer>::value));

        // In-place div with self.
        retval = -5;
        retval /= retval;
        REQUIRE(retval == 1);
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

template <typename T, typename U>
using lshift_t = decltype(std::declval<const T &>() << std::declval<const U &>());

template <typename T, typename U>
using inplace_lshift_t = decltype(std::declval<T &>() <<= std::declval<const U &>());

template <typename T, typename U>
using is_lshiftable = is_detected<lshift_t, T, U>;

template <typename T, typename U>
using is_lshiftable_inplace = is_detected<inplace_lshift_t, T, U>;

template <typename T, typename U>
using rshift_t = decltype(std::declval<const T &>() >> std::declval<const U &>());

template <typename T, typename U>
using inplace_rshift_t = decltype(std::declval<T &>() >>= std::declval<const U &>());

template <typename T, typename U>
using is_rshiftable = is_detected<rshift_t, T, U>;

template <typename T, typename U>
using is_rshiftable_inplace = is_detected<inplace_rshift_t, T, U>;

struct shift_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;
        REQUIRE((lex_cast(ret << 0) == "0"));
        REQUIRE((lex_cast(ret << 1u) == "0"));
        REQUIRE((lex_cast(ret << short(2)) == "0"));
        ret = 1;
        REQUIRE((lex_cast(ret << 1) == "2"));
        REQUIRE((lex_cast(ret << wchar_t{1}) == "2"));
        REQUIRE((lex_cast(ret << 2ll) == "4"));
        ret.neg();
        REQUIRE((lex_cast(ret << 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= char(1)) == "-16"));
        REQUIRE((lex_cast(ret <<= static_cast<signed char>(0)) == "-16"));
        REQUIRE((lex_cast(ret >> 0) == "-16"));
        REQUIRE((lex_cast(ret >> 1) == "-8"));
        REQUIRE((lex_cast(ret >> wchar_t{1}) == "-8"));
        REQUIRE((lex_cast(ret >>= 1ul) == "-8"));
        REQUIRE((lex_cast(ret >>= short(1)) == "-4"));
        REQUIRE((lex_cast(ret >> 128) == "0"));
#if defined(MPPP_HAVE_GCC_INT128)
        ret = 5;
        REQUIRE((ret << __uint128_t{1}) == 10);
        REQUIRE((ret << __int128_t{2}) == 20);
        REQUIRE((ret <<= __uint128_t{1}) == 10);
        REQUIRE((ret <<= __int128_t{2}) == 40);
        REQUIRE((ret >> __uint128_t{1}) == 20);
        REQUIRE((ret >> __int128_t{2}) == 10);
        REQUIRE((ret >>= __uint128_t{1}) == 20);
        REQUIRE((ret >>= __int128_t{2}) == 5);
#endif
        // Error handling.
        REQUIRE_THROWS_AS(ret << -1, std::overflow_error);
        REQUIRE_THROWS_AS(ret <<= -2, std::overflow_error);
        REQUIRE_THROWS_AS(ret >> -1, std::overflow_error);
        REQUIRE_THROWS_AS(ret >>= -2, std::overflow_error);
        if (std::numeric_limits<unsigned long long>::max() > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_AS(ret << std::numeric_limits<unsigned long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret <<= std::numeric_limits<unsigned long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >> std::numeric_limits<unsigned long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >>= std::numeric_limits<unsigned long long>::max(), std::overflow_error);
        }
        if (static_cast<unsigned long long>(std::numeric_limits<long long>::max())
            > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_AS(ret << std::numeric_limits<long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret <<= std::numeric_limits<long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >> std::numeric_limits<long long>::max(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >>= std::numeric_limits<long long>::max(), std::overflow_error);
        }
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE_THROWS_AS(ret << __int128_t{-1}, std::overflow_error);
        REQUIRE_THROWS_AS(ret <<= __int128_t{-1}, std::overflow_error);
        REQUIRE_THROWS_AS(ret >> __int128_t{-1}, std::overflow_error);
        REQUIRE_THROWS_AS(ret >>= __int128_t{-1}, std::overflow_error);
        if (nl_max<__uint128_t>() > nl_max<::mp_bitcnt_t>()) {
            REQUIRE_THROWS_AS(ret << nl_max<__uint128_t>(), std::overflow_error);
            REQUIRE_THROWS_AS(ret <<= nl_max<__uint128_t>(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >> nl_max<__uint128_t>(), std::overflow_error);
            REQUIRE_THROWS_AS(ret >>= nl_max<__uint128_t>(), std::overflow_error);
        }
#endif
        // Type traits.
        REQUIRE((!is_lshiftable<integer, double>::value));
        REQUIRE((!is_lshiftable<integer, integer>::value));
        REQUIRE((!is_lshiftable<integer, std::string>::value));
        REQUIRE((!is_lshiftable<std::string, integer>::value));
        REQUIRE((!is_lshiftable_inplace<integer, std::string>::value));
        REQUIRE((!is_lshiftable_inplace<const integer, int>::value));
        REQUIRE((!is_lshiftable_inplace<std::string, integer>::value));
        REQUIRE((!is_lshiftable_inplace<const int, integer>::value));
        REQUIRE((!is_lshiftable<int, integer>::value));
        REQUIRE((!is_lshiftable_inplace<int, integer>::value));
        REQUIRE((!is_lshiftable_inplace<integer, double>::value));
        REQUIRE((!is_lshiftable_inplace<integer, integer>::value));
        REQUIRE((!is_rshiftable<integer, double>::value));
        REQUIRE((!is_rshiftable<integer, integer>::value));
        REQUIRE((!is_rshiftable<integer, std::string>::value));
        REQUIRE((!is_rshiftable<std::string, integer>::value));
        REQUIRE((!is_rshiftable_inplace<integer, std::string>::value));
        REQUIRE((!is_rshiftable_inplace<const integer, int>::value));
        REQUIRE((!is_rshiftable_inplace<std::string, integer>::value));
        REQUIRE((!is_rshiftable_inplace<const int, integer>::value));
        REQUIRE((!is_rshiftable<int, integer>::value));
        REQUIRE((!is_rshiftable_inplace<int, integer>::value));
        REQUIRE((!is_rshiftable_inplace<integer, double>::value));
        REQUIRE((!is_rshiftable_inplace<integer, integer>::value));
    }
};

TEST_CASE("shift")
{
    tuple_for_each(sizes{}, shift_tester{});
}

template <typename T, typename U>
using mod_t = decltype(std::declval<const T &>() << std::declval<const U &>());

template <typename T, typename U>
using inplace_mod_t = decltype(std::declval<T &>() <<= std::declval<const U &>());

template <typename T, typename U>
using is_modable = is_detected<mod_t, T, U>;

template <typename T, typename U>
using is_modable_inplace = is_detected<inplace_mod_t, T, U>;

struct mod_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 % n2) == "0"));
        REQUIRE((std::is_same<decltype(n1 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % char(3)) == "1"));
        REQUIRE((lex_cast(char(3) % n2) == "1"));
        REQUIRE((lex_cast(n1 % wchar_t(3)) == "1"));
        REQUIRE((lex_cast(wchar_t(3) % n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) % n2), integer>::value));
        REQUIRE((lex_cast(-n1 % static_cast<unsigned char>(3)) == "-1"));
        REQUIRE((lex_cast(static_cast<unsigned char>(3) % n2) == "1"));
        REQUIRE((lex_cast(n1 % short(3)) == "1"));
        REQUIRE((lex_cast(-short(3) % n2) == "-1"));
        REQUIRE((lex_cast(n1 % -3) == "1"));
        REQUIRE((lex_cast(3 % -n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % 3u) == "1"));
        REQUIRE((lex_cast(3u % n2) == "1"));
        REQUIRE((lex_cast(0u % n2) == "0"));
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 % __uint128_t{3} == 1));
        REQUIRE((__uint128_t{3} % integer{-2} == 1));
        REQUIRE((n1 % __int128_t{-3} == 1));
        REQUIRE((__int128_t{-3} % n2 == -1));
#endif
        // In-place mod.
        integer retval{-2};
        retval %= n1;
        REQUIRE((lex_cast(retval) == "-2"));
        retval = 3;
        retval %= 2;
        REQUIRE((lex_cast(retval) == "1"));
        retval = -3;
        retval %= short(2);
        REQUIRE((lex_cast(retval) == "-1"));
        retval %= static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "0"));
        retval = 26;
        retval %= -5ll;
        REQUIRE((lex_cast(retval) == "1"));
        retval = -19;
        retval %= 7ull;
        REQUIRE((lex_cast(retval) == "-5"));
#if defined(MPPP_HAVE_GCC_INT128)
        retval %= __uint128_t{3};
        REQUIRE((retval == -2));
        retval %= __int128_t{2};
        REQUIRE((retval == 0));
#endif
        // CppInteroperable on the left.
        int n = 3;
        n %= integer{2};
        REQUIRE(n == 1);
        n = -3;
        n %= integer{2};
        REQUIRE(n == -1);
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 %= integer{4};
        REQUIRE((n128 == -3));
        __uint128_t un128{6};
        un128 %= integer{5};
        REQUIRE((un128 == 1));
#endif
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} % 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE_THROWS_PREDICATE(integer{1} % __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} % __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
#endif
        REQUIRE((!is_modable<integer, std::string>::value));
        REQUIRE((!is_modable<integer, double>::value));
        REQUIRE((!is_modable<std::string, integer>::value));
        REQUIRE((!is_modable_inplace<integer, std::string>::value));
        REQUIRE((!is_modable_inplace<const integer, int>::value));
        REQUIRE((!is_modable_inplace<std::string, integer>::value));
        REQUIRE((!is_modable_inplace<const int, integer>::value));
        REQUIRE((!is_modable<int, integer>::value));
        REQUIRE((!is_modable_inplace<int, integer>::value));
        // In-place mod with self.
        retval = 5;
        retval %= retval;
        REQUIRE(retval == 0);
    }
};

TEST_CASE("mod")
{
    tuple_for_each(sizes{}, mod_tester{});
}

struct rel_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};

        REQUIRE(n1 != n2);
        REQUIRE(n1 == n1);
        REQUIRE(integer{} == integer{});
        REQUIRE(integer{} == 0);
        REQUIRE(0 == integer{});
        REQUIRE(n1 == 4);
        REQUIRE(4u == n1);
        REQUIRE(n1 == wchar_t{4});
        REQUIRE(wchar_t{4} == n1);
        REQUIRE(n1 != 3);
        REQUIRE(static_cast<signed char>(-3) != n1);
        REQUIRE(4ull == n1);
        REQUIRE(-2 == n2);
        REQUIRE(n2 == short(-2));
        REQUIRE(-2.f == n2);
        REQUIRE(n2 == -2.f);
        REQUIRE(-3.f != n2);
        REQUIRE(n2 != -3.f);
        REQUIRE(-2. == n2);
        REQUIRE(n2 == -2.);
        REQUIRE(-3. != n2);
        REQUIRE(n2 != -3.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(-2.l == n2);
        REQUIRE(n2 == -2.l);
        REQUIRE(-3.l != n2);
        REQUIRE(n2 != -3.l);
        REQUIRE(wchar_t{3} != n2);
        REQUIRE(n2 != wchar_t{3});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(integer{1} == __uint128_t{1});
        REQUIRE(__uint128_t{1} == integer{1});
        REQUIRE(integer{-1} == __int128_t{-1});
        REQUIRE(__int128_t{-1} == integer{-1});
        REQUIRE(integer{0} != __uint128_t{1});
        REQUIRE(__uint128_t{0} != integer{1});
        REQUIRE(integer{-1} != __int128_t{1});
        REQUIRE(__int128_t{1} != integer{-1});
#endif

        REQUIRE(n2 < n1);
        REQUIRE(n2 < 0);
        REQUIRE(-3 < n2);
        REQUIRE(n2 < 0u);
        REQUIRE(n2 < wchar_t{0});
        REQUIRE(wchar_t{0} < n1);
        REQUIRE(-3ll < n2);
        REQUIRE(n2 < 0.f);
        REQUIRE(-3.f < n2);
        REQUIRE(n2 < 0.);
        REQUIRE(-3. < n2);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(n2 < 0.l);
        REQUIRE(-3.l < n2);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(integer{2} < __uint128_t{3});
        REQUIRE(__uint128_t{2} < integer{3});
        REQUIRE(integer{-2} < __int128_t{-1});
        REQUIRE(__int128_t{-2} < integer{-1});
#endif

        REQUIRE(n1 > n2);
        REQUIRE(0 > n2);
        REQUIRE(wchar_t{0} > n2);
        REQUIRE(n1 > wchar_t{0});
        REQUIRE(n2 > -3);
        REQUIRE(0u > n2);
        REQUIRE(n2 > -3ll);
        REQUIRE(0.f > n2);
        REQUIRE(n2 > -3.f);
        REQUIRE(0. > n2);
        REQUIRE(n2 > -3.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(0.l > n2);
        REQUIRE(n2 > -3.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(integer{2} > __uint128_t{1});
        REQUIRE(__uint128_t{2} > integer{1});
        REQUIRE(integer{0} > __int128_t{-1});
        REQUIRE(__int128_t{0} > integer{-1});
#endif

        REQUIRE(n2 <= n1);
        REQUIRE(n1 <= n1);
        REQUIRE(integer{} <= integer{});
        REQUIRE(integer{} <= 0);
        REQUIRE(0 <= integer{});
        REQUIRE(wchar_t{0} <= integer{});
        REQUIRE(integer{} <= wchar_t{0});
        REQUIRE(-2 <= n2);
        REQUIRE(n2 <= -2);
        REQUIRE(n2 <= 0);
        REQUIRE(-3 <= n2);
        REQUIRE(n2 <= 0u);
        REQUIRE(-3ll <= n2);
        REQUIRE(n2 <= 0.f);
        REQUIRE(-3.f <= n2);
        REQUIRE(-2.f <= n2);
        REQUIRE(n2 <= -2.f);
        REQUIRE(n2 <= 0.);
        REQUIRE(-3. <= n2);
        REQUIRE(-2. <= n2);
        REQUIRE(n2 <= -2.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(n2 <= 0.l);
        REQUIRE(-3.l <= n2);
        REQUIRE(-2.l <= n2);
        REQUIRE(n2 <= -2.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(integer{2} <= __uint128_t{3});
        REQUIRE(__uint128_t{2} <= integer{2});
        REQUIRE(integer{-2} <= __int128_t{-1});
        REQUIRE(__int128_t{-2} <= integer{-2});
#endif

        REQUIRE(n1 >= n2);
        REQUIRE(n1 >= n1);
        REQUIRE(integer{} >= integer{});
        REQUIRE(integer{} >= 0);
        REQUIRE(integer{} >= wchar_t{0});
        REQUIRE(0 >= integer{});
        REQUIRE(wchar_t{0} >= integer{});
        REQUIRE(-2 >= n2);
        REQUIRE(n2 >= -2);
        REQUIRE(0 >= n2);
        REQUIRE(n2 >= -3);
        REQUIRE(0u >= n2);
        REQUIRE(n2 >= -3ll);
        REQUIRE(0.f >= n2);
        REQUIRE(n2 >= -3.f);
        REQUIRE(-2.f >= n2);
        REQUIRE(n2 >= -2.f);
        REQUIRE(0. >= n2);
        REQUIRE(n2 >= -3.);
        REQUIRE(-2. >= n2);
        REQUIRE(n2 >= -2.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(0.l >= n2);
        REQUIRE(n2 >= -3.l);
        REQUIRE(-2.l >= n2);
        REQUIRE(n2 >= -2.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(integer{2} >= __uint128_t{1});
        REQUIRE(__uint128_t{2} >= integer{2});
        REQUIRE(integer{0} >= __int128_t{-1});
        REQUIRE(__int128_t{0} >= integer{0});
#endif
    }
};

TEST_CASE("rel")
{
    tuple_for_each(sizes{}, rel_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
