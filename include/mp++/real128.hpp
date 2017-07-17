// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL128_HPP
#define MPPP_REAL128_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/quadmath.hpp>
#include <mp++/integer.hpp>

namespace mppp
{

class real128
{
    // Number of digits in the significand (113).
    static constexpr unsigned sig_digits = FLT128_MANT_DIG;

public:
    constexpr real128() : m_value(0)
    {
    }
    constexpr explicit real128(::__float128 x) : m_value(x)
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    constexpr explicit real128(CppInteroperable x)
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
    constexpr explicit real128(T x)
#endif
        : m_value(x)
    {
    }
    template <std::size_t SSize>
    explicit real128(const integer<SSize> &n)
    {
        // Special case for zero.
        const auto n_sgn = n.sgn();
        if (!n_sgn) {
            m_value = 0;
            return;
        }
        // Get the pointer to the limbs, and the size in limbs and bits.
        const ::mp_limb_t *ptr = n.is_static() ? n._get_union().g_st().m_limbs.data() : n._get_union().g_dy()._mp_d;
        const std::size_t n_bits = n.nbits();
        // Let's get the size in limbs from the size in bits, as we already made the effort.
        std::size_t ls = n_bits / unsigned(GMP_NUMB_BITS) + static_cast<bool>(n_bits % unsigned(GMP_NUMB_BITS));
        assert(ls && n_bits && ls == n.size());

        // Init value with the most significand limb, and move to the next limb.
        m_value = ptr[--ls] & GMP_NUMB_MASK;
        // Number of bits read so far from n: it is the size in bits of the top limb.
        auto read_bits = static_cast<unsigned>((n_bits % unsigned(GMP_NUMB_BITS)) ? (n_bits % unsigned(GMP_NUMB_BITS))
                                                                                  : unsigned(GMP_NUMB_BITS));
        assert(read_bits);
        // Keep on reading as long as we have limbs and as long as we haven't read enough bits
        // to fill up the significand of m_value.
        // NOTE: a paranoia check about possible overflow in read_bits += rbits.
        static_assert(sig_digits <= std::numeric_limits<unsigned>::max() - unsigned(GMP_NUMB_BITS), "Overflow error.");
        while (ls && read_bits < sig_digits) {
            // Number of bits to be read from the current limb: GMP_NUMB_BITS or less.
            const unsigned rbits = std::min(unsigned(GMP_NUMB_BITS), sig_digits - read_bits);
            // Shift m_value by rbits.
            m_value = ::scalbnq(m_value, static_cast<int>(rbits));
            // Add the bottom part, and move to the next limb. We might need to remove lower bits
            // in case rbits is not exactly GMP_NUMB_BITS.
            m_value += (ptr[--ls] & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - rbits);
            // Update the number of read bits.
            read_bits += rbits;
        }
        if (read_bits < n_bits) {
            // We have extra bits in n, we will have to shift m_value accordingly.
            // Use the long variant to maximise the range.
            if (mppp_unlikely(n_bits - read_bits > static_cast<unsigned long>(std::numeric_limits<long>::max()))) {
                throw std::overflow_error(
                    "Overflow in the construction of a real128 from an integer: the second argument to scalblnq() is "
                    + std::to_string(n_bits - read_bits) + ", but the max allowed value is "
                    + std::to_string(std::numeric_limits<long>::max()));
            }
            m_value = ::scalblnq(m_value, static_cast<long>(n_bits - read_bits));
        }
        // Fix the sign as needed.
        if (n_sgn == -1) {
            m_value = -m_value;
        }
    }
    real128(const real128 &) = default;
    real128(real128 &&) = default;
    real128 &operator=(const real128 &) = default;
    real128 &operator=(real128 &&) = default;
    // NOTE: drop the "::" here as it confuses clang-format.
    MPPP_MAYBE_CONSTEXPR __float128 &value()
    {
        return m_value;
    }
    MPPP_MAYBE_CONSTEXPR const ::__float128 &value() const
    {
        return m_value;
    }

private:
    ::__float128 m_value;
};
}

#else

#error The real128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
