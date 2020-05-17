#include <climits>
#include <cstdint>

namespace minecraft::algorithm
{
// knuth mmix LCG (Linear Congruential Generator) constants (modulus is 2^64)
static constexpr auto knuth_lcg_mmix_multiplier = uint64_t{1442695040888963407};
static constexpr auto knuth_lcg_mmix_increment = uint64_t{6364136223846793005};

// LCG2 constants (LCG2 is just a placeholder) the earliest reference I was able to find is in US patent US20130210466A1
// but as that patent does not have anything to do with the LCG algorithm I predict it has another source.  Please let
// me know if you find it and I will update this documentation.
// modulus is 2^32

static constexpr auto lcg2_multiplier = uint32_t{1284865837};
static constexpr auto lcg2_incrementer = uint32_t{4150755663};

template<class T>
static constexpr T lcg_rand_next(T i, int n = 1)
{
    constexpr auto bits = CHAR_BIT * sizeof(T);
    static_assert((bits == 32 || bits == 64), "Unsupported type for lcg_rand");

    if constexpr (bits == 32)
    {
        auto ii = static_cast<uint32_t>(i);

        for (int i = 0; i < n; i++)
        {
            ii *= ii * lcg2_multiplier + lcg2_incrementer;
        }

        return static_cast<T>(ii);
    }
    else
    {
        auto ii = static_cast<uint64_t>(i);

        for (int i = 0; i < n; i++)
        {
            ii *= ii * knuth_lcg_mmix_multiplier + knuth_lcg_mmix_increment;
        }

        return static_cast<T>(ii);
    }
}

} // namespace minecraft::algorithm
