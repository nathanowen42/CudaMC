#pragma once

#include <array>
#include <cstdint>
#include <limits>

namespace minecraft::biome
{
using biome_integral_t = int;
static constexpr biome_integral_t MUTATED_BIOME_OFFSET = 128;

constexpr biome_integral_t biome_mask = 0xff;
constexpr biome_integral_t max_biomes = 256;

enum class biome_id : biome_integral_t
{
    none = -1,

    // 0
    ocean = 0,
    plains,
    desert,
    mountains,
    forest,
    taiga,
    swamp,
    river,
    nether_wastes,
    the_end,

    // 10
    frozen_ocean,
    frozen_river,
    snowy_tundra,
    snowy_mountains,
    mushroom_fields,
    mushroom_field_shore,
    beach,
    desert_hills,
    wooded_hills,
    taiga_hills,

    // 20
    mountain_edge,
    jungle,
    jungle_hills,
    jungle_edge,
    deep_ocean,
    stone_shore,
    snowy_beach,
    birch_forest,
    birch_forest_hills,
    dark_forest,

    // 30
    snowy_taiga,
    snowy_taiga_hills,
    giant_tree_taiga,
    giant_tree_taiga_hills,
    wooded_mountains,
    savanna,
    savanna_plateau,
    badlands,
    wooded_badlands_plateau,
    badlands_plateau,

    // 40  --  1.13
    small_end_islands,
    end_midlands,
    end_highlands,
    end_barrens,
    warm_ocean,
    lukewarm_ocean,
    cold_ocean,
    deep_warm_ocean,
    deep_lukewarm_ocean,
    deep_cold_ocean,

    // 50
    deep_frozen_ocean,
    // BIOME_NUM,

    the_void = 127,

    // 1.14
    bamboo_jungle = 168,
    bamboo_jungle_hills = 169,

    // 1.16
    soul_sand_valley = 170,
    crimson_forest = 171,
    warped_forest = 172,
    basalt_deltas = 173,

    // technical biomes
    sunflower_plains = plains + MUTATED_BIOME_OFFSET, // 129
    desert_lakes = desert + MUTATED_BIOME_OFFSET, // 130
    gravelly_mountains = mountains + MUTATED_BIOME_OFFSET, // 131
    flower_forest = forest + MUTATED_BIOME_OFFSET, // 132
    taiga_mountains = taiga + MUTATED_BIOME_OFFSET, // 133
    swamp_hills = swamp + MUTATED_BIOME_OFFSET, // 134
    ice_spikes = snowy_tundra + MUTATED_BIOME_OFFSET, // 140
    modified_jungle = jungle + MUTATED_BIOME_OFFSET, // 149
    modified_jungle_edge = jungle_edge + MUTATED_BIOME_OFFSET, // 151
    tall_birch_forest = birch_forest + MUTATED_BIOME_OFFSET, // 155
    tall_birch_hills = birch_forest_hills + MUTATED_BIOME_OFFSET, // 156
    dark_forest_hills = dark_forest + MUTATED_BIOME_OFFSET, // 157
    snowy_taiga_mountains = snowy_taiga + MUTATED_BIOME_OFFSET, // 158
    giant_spruce_taiga = giant_tree_taiga + MUTATED_BIOME_OFFSET, // 160
    giant_spruce_taiga_hills = giant_tree_taiga_hills + MUTATED_BIOME_OFFSET, // 161
    modified_gravelly_mountains = wooded_mountains + MUTATED_BIOME_OFFSET, // 162
    shattered_savanna = savanna + MUTATED_BIOME_OFFSET, // 163
    shattered_savanna_plateau = savanna_plateau + MUTATED_BIOME_OFFSET, // 164
    eroded_badlands = badlands + MUTATED_BIOME_OFFSET, // 165
    modified_wooded_badlands_plateau = wooded_badlands_plateau + MUTATED_BIOME_OFFSET, // 166
    modified_badlands_plateau = badlands_plateau + MUTATED_BIOME_OFFSET, // 167
};

enum class biome_type
{
    the_void = -1,
    ocean,
    plains,
    desert,
    hills,
    forest,
    taiga,
    swamp,
    river,
    nether,
    sky,
    snow,
    mushroom_island,
    beach,
    jungle,
    stone_beach,
    savanna,
    mesa,
    BTYPE_NUM
};

enum temp_catagory
{
    the_void,
    oceanic,
    warm,
    lush,
    cold,
    freezing,
    special
};

constexpr double default_temp = 0.5;

struct biome
{
    biome_id id;
    biome_type type;
    double height;
    double temp;
    int temp_catagory;
    biome_id mutated;

    constexpr biome()
        : id{biome_id::none},
          type{biome_type::the_void},
          temp{default_temp},
          height{0},
          temp_catagory{temp_catagory::the_void},
          mutated{biome_id::none}
    {
    }

    constexpr biome(const biome& other) = default;
};

using biome_array = std::array<biome, max_biomes>;

static constexpr biome_array populate_biomes()
{
    biome_array biome_list{};

    auto add_biome = [&](biome_id id, temp_catagory temp_cat, biome_type type, double temp, double height) constexpr
    {
        auto& biome = biome_list[static_cast<biome_integral_t>(id)];
        biome.id = id;
        biome.temp_catagory = temp_cat;
        biome.type = type;
        biome.temp = temp;
        biome.height = height;
    };

    auto add_mutation = [&](biome_id id) constexpr
    {
        auto mutated_id = static_cast<biome_id>(static_cast<biome_integral_t>(id) + MUTATED_BIOME_OFFSET);
        biome_list[static_cast<biome_integral_t>(id)].mutated = mutated_id;
        biome_list[static_cast<biome_integral_t>(mutated_id)] = biome_list[static_cast<uint8_t>(id)];
        biome_list[static_cast<biome_integral_t>(mutated_id)].id = mutated_id;
    };

    constexpr double default_height = 0.1;
    constexpr double shallow_waters_height = -0.5;
    constexpr double oceans_height = -1.0;
    constexpr double deep_oceans_height = -1.8;
    constexpr double low_plains_height = 0.125;
    constexpr double mid_plains_height = 0.2;
    constexpr double low_hills_height = 0.45;
    constexpr double high_plateaus_height = 1.5;
    constexpr double mid_hills_height = 1.0;
    constexpr double shores_height = 0.0;
    constexpr double rocky_waters_height = 0.1;
    constexpr double low_islands_height = 0.2;
    constexpr double partially_submerged_height = -0.2;

    add_biome(biome_id::ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, oceans_height);
    add_biome(biome_id::plains, temp_catagory::lush, biome_type::plains, 0.8, default_height);
    add_biome(biome_id::desert, temp_catagory::warm, biome_type::desert, 2.0, low_plains_height);
    add_biome(biome_id::mountains, temp_catagory::lush, biome_type::hills, 0.2, mid_hills_height);
    add_biome(biome_id::forest, temp_catagory::lush, biome_type::forest, 0.7, default_height);
    add_biome(biome_id::taiga, temp_catagory::lush, biome_type::taiga, 0.25, mid_plains_height);
    add_biome(biome_id::swamp, temp_catagory::lush, biome_type::swamp, 0.8, partially_submerged_height);
    add_biome(biome_id::river, temp_catagory::lush, biome_type::river, 0.5, shallow_waters_height);
    add_biome(biome_id::nether_wastes, temp_catagory::warm, biome_type::nether, 2.0, default_height);
    add_biome(biome_id::the_end, temp_catagory::lush, biome_type::sky, 0.5, default_height);
    add_biome(biome_id::frozen_ocean, temp_catagory::oceanic, biome_type::ocean, 0.0, oceans_height);
    add_biome(biome_id::frozen_river, temp_catagory::cold, biome_type::river, 0.0, shallow_waters_height);
    add_biome(biome_id::snowy_tundra, temp_catagory::cold, biome_type::snow, 0.0, low_plains_height);
    add_biome(biome_id::snowy_mountains, temp_catagory::cold, biome_type::snow, 0.0, low_hills_height);
    add_biome(biome_id::mushroom_fields, temp_catagory::lush, biome_type::mushroom_island, 0.9, low_islands_height);
    add_biome(biome_id::mushroom_field_shore, temp_catagory::lush, biome_type::mushroom_island, 0.9, shores_height);
    add_biome(biome_id::beach, temp_catagory::lush, biome_type::beach, 0.8, shores_height);
    add_biome(biome_id::desert_hills, temp_catagory::warm, biome_type::desert, 2.0, low_hills_height);
    add_biome(biome_id::wooded_hills, temp_catagory::lush, biome_type::forest, 0.7, low_hills_height);
    add_biome(biome_id::taiga_hills, temp_catagory::lush, biome_type::taiga, 0.25, low_hills_height);
    add_biome(biome_id::mountain_edge, temp_catagory::lush, biome_type::hills, 0.2, mid_hills_height);
    add_biome(biome_id::jungle, temp_catagory::lush, biome_type::jungle, 0.95, default_height);
    add_biome(biome_id::jungle_hills, temp_catagory::lush, biome_type::jungle, 0.95, low_hills_height);
    add_biome(biome_id::jungle_edge, temp_catagory::lush, biome_type::jungle, 0.95, default_height);
    add_biome(biome_id::deep_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, deep_oceans_height);
    add_biome(biome_id::stone_shore, temp_catagory::lush, biome_type::stone_beach, 0.2, rocky_waters_height);
    add_biome(biome_id::snowy_beach, temp_catagory::cold, biome_type::beach, 0.05, shores_height);
    add_biome(biome_id::birch_forest, temp_catagory::lush, biome_type::forest, 0.6, default_height);
    add_biome(biome_id::birch_forest_hills, temp_catagory::lush, biome_type::forest, 0.6, low_hills_height);
    add_biome(biome_id::dark_forest, temp_catagory::lush, biome_type::forest, 0.7, default_height);
    add_biome(biome_id::snowy_taiga, temp_catagory::cold, biome_type::taiga, -0.5, mid_plains_height);
    add_biome(biome_id::snowy_taiga_hills, temp_catagory::cold, biome_type::taiga, -0.5, low_hills_height);
    add_biome(biome_id::giant_tree_taiga, temp_catagory::lush, biome_type::taiga, 0.3, mid_plains_height);
    add_biome(biome_id::giant_tree_taiga_hills, temp_catagory::lush, biome_type::taiga, 0.3, low_hills_height);
    add_biome(biome_id::wooded_mountains, temp_catagory::lush, biome_type::hills, 0.2, mid_hills_height);
    add_biome(biome_id::savanna, temp_catagory::warm, biome_type::savanna, 1.2, low_plains_height);
    add_biome(biome_id::savanna_plateau, temp_catagory::warm, biome_type::savanna, 1.0, high_plateaus_height);
    add_biome(biome_id::badlands, temp_catagory::warm, biome_type::mesa, 2.0, default_height);
    add_biome(biome_id::wooded_badlands_plateau, temp_catagory::warm, biome_type::mesa, 2.0, high_plateaus_height);
    add_biome(biome_id::badlands_plateau, temp_catagory::warm, biome_type::mesa, 2.0, high_plateaus_height);

    add_biome(biome_id::small_end_islands, temp_catagory::lush, biome_type::sky, 0.5, default_height);
    add_biome(biome_id::end_midlands, temp_catagory::lush, biome_type::sky, 0.5, default_height);
    add_biome(biome_id::end_highlands, temp_catagory::lush, biome_type::sky, 0.5, default_height);
    add_biome(biome_id::end_barrens, temp_catagory::lush, biome_type::sky, 0.5, default_height);
    add_biome(biome_id::warm_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, oceans_height);
    add_biome(biome_id::lukewarm_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, oceans_height);
    add_biome(biome_id::cold_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, oceans_height);
    add_biome(biome_id::deep_warm_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, deep_oceans_height);
    add_biome(biome_id::deep_lukewarm_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, deep_oceans_height);
    add_biome(biome_id::deep_cold_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, deep_oceans_height);
    add_biome(biome_id::deep_frozen_ocean, temp_catagory::oceanic, biome_type::ocean, 0.5, deep_oceans_height);

    add_biome(biome_id::the_void, temp_catagory::the_void, biome_type::the_void, 0.5, 0);

    add_biome(biome_id::bamboo_jungle, temp_catagory::lush, biome_type::jungle, 0.95, default_height);
    add_biome(biome_id::bamboo_jungle_hills, temp_catagory::lush, biome_type::jungle, 0.95, low_hills_height);

    add_biome(biome_id::soul_sand_valley, temp_catagory::warm, biome_type::nether, 2.0, default_height);
    add_biome(biome_id::crimson_forest, temp_catagory::warm, biome_type::nether, 2.0, default_height);
    add_biome(biome_id::warped_forest, temp_catagory::warm, biome_type::nether, 2.0, default_height);
    add_biome(biome_id::basalt_deltas, temp_catagory::warm, biome_type::nether, 2.0, default_height);

    add_mutation(biome_id::plains);
    add_mutation(biome_id::desert);
    add_mutation(biome_id::mountains);
    add_mutation(biome_id::forest);
    add_mutation(biome_id::taiga);
    add_mutation(biome_id::swamp);
    add_mutation(biome_id::snowy_tundra);
    add_mutation(biome_id::jungle);
    add_mutation(biome_id::jungle_edge);
    add_mutation(biome_id::birch_forest);
    add_mutation(biome_id::birch_forest_hills);
    add_mutation(biome_id::dark_forest);
    add_mutation(biome_id::snowy_taiga);
    add_mutation(biome_id::giant_tree_taiga);
    add_mutation(biome_id::giant_tree_taiga_hills);
    add_mutation(biome_id::wooded_mountains);
    add_mutation(biome_id::savanna);
    add_mutation(biome_id::savanna_plateau);
    add_mutation(biome_id::badlands);
    add_mutation(biome_id::wooded_badlands_plateau);
    add_mutation(biome_id::badlands_plateau);

    return biome_list;
}

static constexpr auto biomes = populate_biomes();

static constexpr biome_type get_biome_type(int id)
{
    return static_cast<bool>(id & ~biome_mask) ? biome_type::the_void : biomes[id].type;
}

static constexpr int biome_exists(int id)
{
    return (!(id & ~biome_mask) && (biomes[id].id != biome_id::none));
}

static constexpr int get_temperature_category(int id)
{
    return (id & ~biome_mask) ? temp_catagory::the_void : biomes[id].temp_catagory;
}

static constexpr int are_similar(int id1, int id2)
{
    auto id1_biome = static_cast<biome_id>(id1);
    auto id2_biome = static_cast<biome_id>(id2);

    if (id1 == id2)
    {
        return 1;
    }

    if (id1_biome == biome_id::wooded_badlands_plateau || id1_biome == biome_id::badlands_plateau)
    {
        return id2_biome == biome_id::wooded_badlands_plateau || id2_biome == biome_id::badlands_plateau;
    }

    // adjust for asymmetric equality (workaround to simulate a bug in the MC java code)
    if (id1 >= MUTATED_BIOME_OFFSET || id2 >= MUTATED_BIOME_OFFSET)
    {
        // skip biomes that did not overload the isEqualTo() method
        switch (id2_biome)
        {
            case biome_id::desert_lakes:
                [[fallthrough]];
            case biome_id::taiga_mountains:
                [[fallthrough]];
            case biome_id::swamp_hills:
                [[fallthrough]];
            case biome_id::modified_jungle:
                [[fallthrough]];
            case biome_id::modified_jungle_edge:
                [[fallthrough]];
            case biome_id::tall_birch_forest:
                [[fallthrough]];
            case biome_id::tall_birch_hills:
                [[fallthrough]];
            case biome_id::dark_forest_hills:
                [[fallthrough]];
            case biome_id::snowy_taiga_mountains:
                [[fallthrough]];
            case biome_id::shattered_savanna:
                [[fallthrough]];
            case biome_id::shattered_savanna_plateau:
                return 0;
            default:
                break;
        }
    }

    if (!biome_exists(id1) || !biome_exists(id2))
    {
        return 0;
    }

    return get_biome_type(id1) == get_biome_type(id2);
}

bool is_same(int i1, int i2)
{
    return are_similar(i1, i2);
}

template<biome_id... Args>
constexpr uint64_t create_catagory_mask()
{
    static_assert((... && (static_cast<uint64_t>(Args) < std::numeric_limits<uint64_t>::digits)));
    return (... | (uint64_t{1} << static_cast<uint64_t>(Args)));
}

static constexpr int is_shallow_ocean(int id)
{
    constexpr uint64_t shallow_bits = create_catagory_mask<biome_id::ocean, biome_id::frozen_ocean,
        biome_id::warm_ocean, biome_id::lukewarm_ocean, biome_id::cold_ocean>();
    return id < 64 && ((uint64_t{1} << id) & shallow_bits);
}

static constexpr int is_deep_ocean(int id)
{
    constexpr uint64_t deep_bits = create_catagory_mask<biome_id::deep_ocean, biome_id::deep_warm_ocean,
        biome_id::deep_lukewarm_ocean, biome_id::deep_cold_ocean, biome_id::deep_frozen_ocean>();
    return id < 64 && ((uint64_t{1} << id) & deep_bits);
}

static constexpr int is_oceanic(int id)
{
    constexpr uint64_t ocean_bits = create_catagory_mask<biome_id::ocean, biome_id::frozen_ocean, biome_id::warm_ocean,
        biome_id::lukewarm_ocean, biome_id::cold_ocean, biome_id::deep_ocean, biome_id::deep_warm_ocean,
        biome_id::deep_lukewarm_ocean, biome_id::deep_cold_ocean, biome_id::deep_frozen_ocean>();
    return id < 64 && ((uint64_t{1} << id) & ocean_bits);
}

static constexpr int is_biome_snowy(int id)
{
    return biome_exists(id) && biomes[id].temp < 0.1;
}

} // namespace minecraft::biome
