/*
 * Copyright (c) 1995, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

//Note this code is pulled from OpenJDK src/share/classes/java/util/Random.java @ 9107:687fd7c7986d
//and converted to functional C++

#pragma once

#include <algorithm>
#include <atomic>
#include <limits>

namespace java{

class random
{
public:
    random(int64_t seed_) : seed((seed_ ^ multiplier) & mask)
    {
    }

    /**
     * Generates the next pseudorandom number. Subclasses should
     * override this, as this is used by all other methods.
     *
     * The general contract of {@code next} is that it returns an
     * {@code int} value and if the argument {@code bits} is between
     * {@code 1} and {@code 32} (inclusive), then that many low-order
     * bits of the returned value will be (approximately) independently
     * chosen bit values, each of which is (approximately) equally
     * likely to be {@code 0} or {@code 1}. The method {@code next} is
     * implemented by class {@code Random} by atomically updating the seed to
     *  {@code (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1)}
     * and returning
     *  {@code (int)(seed >>> (48 - bits))}.
     *
     * This is a linear congruential pseudorandom number generator, as
     * defined by D. H. Lehmer and described by Donald E. Knuth in
     * gThe Art of Computer Programming, Volume 3:
     * gSeminumerical Algorithms, section 3.2.1.
     *
     * @template param bits random bits
     * @return the next pseudorandom value from this random number
     *         generator's sequence
     * @since  1.1
     */
    template <unsigned bits>
    inline int next() noexcept
    {
        static_assert(bits <= std::numeric_limits<unsigned int>::digits, "");
        int64_t oldseed, nextseed;

        do
        {
            oldseed = seed.load();
            nextseed = (oldseed * multiplier + addend) & mask;
        } while (!seed.compare_exchange_strong(oldseed, nextseed));

        return static_cast<int>(static_cast<uint64_t>(nextseed) >> (48 - bits));
    }


    /**
     * Returns the next pseudorandom, uniformly distributed {@code int}
     * value from this random number generator's sequence. The general
     * contract of {@code nextInt} is that one {@code int} value is
     * pseudorandomly generated and returned. All 232 possible
     * {@code int} values are produced with (approximately) equal probability.
     *
     * The method {@code nextInt} is implemented by class {@code Random}
     * as if by:
     *   {@code
     * int nextInt() {
     *   return next<32>();
     * }}
     *
     * @return the next pseudorandom, uniformly distributed {@code int}
     *         value from this random number generator's sequence
     */
    inline int next_int() noexcept
    {
        return next<32>();
    }

    /**
     * Returns a pseudorandom, uniformly distributed {@code int} value
     * between 0 (inclusive) and the specified value (exclusive), drawn from
     * this random number generator's sequence.  The general contract of
     * {@code nextInt} is that one {@code int} value in the specified range
     * is pseudorandomly generated and returned.  All {@code bound} possible
     * {@code int} values are produced with (approximately) equal
     * probability.  The method {@code nextInt(int bound)} is implemented by
     * class {@code Random} as if by:
     *   {@code
     * int nextInt(int bound) {
     *   if (bound <= 0)
     *     throw new IllegalArgumentException("bound must be positive");
     *
     *   if ((bound & -bound) == bound)  // i.e., bound is a power of 2
     *     return (int)((bound * (long)next<31>()) >> 31);
     *
     *   int bits, val;
     *   do {
     *       bits = next<31>();
     *       val = bits % bound;
     *   } while (bits - val + (bound-1) < 0);
     *   return val;
     * }}
     *
     * The hedge "approximately" is used in the foregoing description only
     * because the next method is only approximately an unbiased source of
     * independently chosen bits.  If it were a perfect source of randomly
     * chosen bits, then the algorithm shown would choose {@code int}
     * values from the stated range with perfect uniformity.
     * 
     * The algorithm is slightly tricky.  It rejects values that would result
     * in an uneven distribution (due to the fact that 2^31 is not divisible
     * by n). The probability of a value being rejected depends on n.  The
     * worst case is n=2^30+1, for which the probability of a reject is 1/2,
     * and the expected number of iterations before the loop terminates is 2.
     * 
     * The algorithm treats the case where n is a power of two specially: it
     * returns the correct number of high-order bits from the underlying
     * pseudo-random number generator.  In the absence of special treatment,
     * the correct number of glow-order bits would be returned.  Linear
     * congruential pseudo-random number generators such as the one
     * implemented by this class are known to have short periods in the
     * sequence of values of their low-order bits.  Thus, this special case
     * greatly increases the length of the sequence of values returned by
     * successive calls to this method if n is a small power of two.
     *
     * @param bound the upper bound (exclusive).  Must be positive.
     * @return the next pseudorandom, uniformly distributed {@code int}
     *         value between zero (inclusive) and {@code bound} (exclusive)
     *         from this random number generator's sequence
     * @throws IllegalArgumentException if bound is not positive
     * @since 1.2
     */
    inline int next_int(unsigned int bound) noexcept
    {
        /*
         * note in the Java code they pass an int for bounds (there are no unsigned values
         * in Java) and throw if it is negative.  Rather than make this function throw
         * I have chosen to take an unsigned int and, if necessary, reduce it to INT_MAX.
         * This will yield the same results as the Java code for all valid values and 
         * removes the need to throw.  It does however mean that this will not catch errors
         * resulting from values above INT_MAX.  As the intent here is to duplicate the Java
         * random results for well-behaved code known to work in Java, and not to create a good
         * random number generator this should be acceptable.  Any discrepancies are a result
         * of incorrect calls to this function.
         */

        bound = std::min(static_cast<unsigned int>(std::numeric_limits<int>::max()), bound);

        int r = next<31>();
        int m = bound - 1;
        
        if ((bound & m) == 0) // i.e., bound is a power of 2  
        {
            r = static_cast<int>((bound * static_cast<int64_t>(r)) >> 31);
        }
        else 
        {
            for (int u = r; u - (r = u % bound) + m < 0; u = next<31>());
        }

        return r;
    }

    /**
     * Returns the next pseudorandom, uniformly distributed {@code long}
     * value from this random number generator's sequence. The general
     * contract of {@code nextLong} is that one {@code long} value is
     * pseudorandomly generated and returned.
     *
     * The method {@code nextLong} is implemented by class {@code Random}
     * as if by:
     *   {@code
     * long nextLong() {
     *   return ((long)next<32>() << 32) + next<32>();
     * }}
     *
     * Because class {@code Random} uses a seed with only 48 bits,
     * this algorithm will not return all possible {@code long} values.
     *
     * @return the next pseudorandom, uniformly distributed {@code long}
     *         value from this random number generator's sequence
     */
    inline long next_long() noexcept
    {
        // it's okay that the bottom word remains signed.
        return ((long)(next<32>()) << 32) + next<32>();
    }

    /**
     * Returns the next pseudorandom, uniformly distributed
     * {@code boolean} value from this random number generator's
     * sequence. The general contract of {@code nextBoolean} is that one
     * {@code boolean} value is pseudorandomly generated and returned.  The
     * values {@code true} and {@code false} are produced with
     * (approximately) equal probability.
     *
     * The method {@code nextBoolean} is implemented by class {@code Random}
     * as if by:
     *   {@code
     * boolean nextBoolean() {
     *   return next<1>() != 0;
     * }}
     *
     * @return the next pseudorandom, uniformly distributed
     *         {@code boolean} value from this random number generator's
     *         sequence
     * @since 1.2
     */
    inline bool next_bool() noexcept
    {
        return next<1>() != 0;
    }

    /**
     * Returns the next pseudorandom, uniformly distributed {@code float}
     * value between {@code 0.0} and {@code 1.0} from this random
     * number generator's sequence.
     *
     * The general contract of {@code nextFloat} is that one
     * {@code float} value, chosen (approximately) uniformly from the
     * range {@code 0.0f} (inclusive) to {@code 1.0f} (exclusive), is
     * pseudorandomly generated and returned. All 2^24 possible
     * {@code float} values of the form 2^24,
     * where m is a positive integer less than 224, are
     * produced with (approximately) equal probability.
     *
     * The method {@code nextFloat} is implemented by class {@code Random}
     * as if by:
     *   {@code
     * public float nextFloat() {
     *   return next<24>() / ((float)(1 << 24));
     * }}
     *
     * The hedge "approximately" is used in the foregoing description only
     * because the next method is only approximately an unbiased source of
     * independently chosen bits. If it were a perfect source of randomly
     * chosen bits, then the algorithm shown would choose {@code float}
     * values from the stated range with perfect uniformity.
     * [In early versions of Java, the result was incorrectly calculated as:
     *   {@code
     *   return next(30) / ((float)(1 << 30));}
     * This might seem to be equivalent, if not better, but in fact it
     * introduced a slight nonuniformity because of the bias in the rounding
     * of floating-point numbers: it was slightly more likely that the
     * low-order bit of the significand would be 0 than that it would be 1.]
     *
     * @return the next pseudorandom, uniformly distributed {@code float}
     *         value between {@code 0.0} and {@code 1.0} from this
     *         random number generator's sequence
     */
    inline float next_float() noexcept
    {
        return next<24>() / (static_cast<float>(1 << 24));
    }

    /**
     * Returns the next pseudorandom, uniformly distributed
     * {@code double} value between {@code 0.0} and
     * {@code 1.0} from this random number generator's sequence.
     *
     * The general contract of {@code nextDouble} is that one
     * {@code double} value, chosen (approximately) uniformly from the
     * range {@code 0.0d} (inclusive) to {@code 1.0d} (exclusive), is
     * pseudorandomly generated and returned.
     *
     * The method {@code nextDouble} is implemented by class {@code Random}
     * as if by:
     *   {@code
     * public double nextDouble() {
     *   return (((long)next<26>() << 27) + next<27>())
     *     / (double)(1L << 53);
     * }}
     *
     * The hedge "approximately" is used in the foregoing description only
     * because the {@code next} method is only approximately an unbiased
     * source of independently chosen bits. If it were a perfect source of
     * randomly chosen bits, then the algorithm shown would choose
     * {@code double} values from the stated range with perfect uniformity.
     * [In early versions of Java, the result was incorrectly calculated as:
     *   {@code
     *   return (((long)next<27>() << 27) + next<27>())
     *     / (double)(1L << 54);}
     * This might seem to be equivalent, if not better, but in fact it
     * introduced a large nonuniformity because of the bias in the rounding
     * of floating-point numbers: it was three times as likely that the
     * low-order bit of the significand would be 0 than that it would be 1!
     * This nonuniformity probably doesn't matter much in practice, but we
     * strive for perfection.]
     *
     * @return the next pseudorandom, uniformly distributed {@code double}
     *         value between {@code 0.0} and {@code 1.0} from this
     *         random number generator's sequence
     * @see Math#random
     */
    inline double next_double() noexcept
    {
        return ((static_cast<int64_t>(next<26>()) << 27) + next<27>()) * DOUBLE_UNIT;
    }


private:
    static constexpr double DOUBLE_UNIT = static_cast<double>(uint64_t{1} << 53);
    static constexpr int64_t multiplier = 0x5DEECE66DL;
    static constexpr int64_t addend = int64_t{0xB};
    static constexpr int64_t mask = (int64_t{1} << 48) - 1;

    std::atomic<int64_t> seed;
};

}
