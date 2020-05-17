#include "biome_generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace minecraft::biome_generator
{
world_layer::world_layer(int scale_, world_layer* parent1_, world_layer* parent2_, int seed)
    : base_seed{get_base_seed(seed)},
      world_seed{get_world_seed(seed)},
      scale{scale_},
      parent1{parent1_},
      parent2{parent2_}
{
}

int64_t world_layer::get_chunk_seed(int64_t chunk_x, int64_t chunk_z)
{
    auto chunk_seed = world_seed;
    chunk_seed = algorithm::lcg_rand_next(chunk_seed);
    chunk_seed += chunk_x;
    chunk_seed = algorithm::lcg_rand_next(chunk_seed);
    chunk_seed += chunk_z;
    chunk_seed = algorithm::lcg_rand_next(chunk_seed);
    chunk_seed += chunk_x;
    chunk_seed = algorithm::lcg_rand_next(chunk_seed);
    chunk_seed += chunk_z;
    return chunk_seed;
}

int64_t world_layer::get_base_seed(int64_t seed)
{
    auto baseSeed = seed;
    baseSeed = algorithm::lcg_rand_next(baseSeed);
    baseSeed += seed;
    baseSeed = algorithm::lcg_rand_next(baseSeed);
    baseSeed += seed;
    baseSeed = algorithm::lcg_rand_next(baseSeed);
    baseSeed += seed;
    return baseSeed;
}

int64_t world_layer::get_world_seed(int64_t seed)
{
    if (p2 != nullptr && get_map != world_layer::map_hills)
    {
        get_world_seed(p2, seed);
    }

    if (p != nullptr)
    {
        get_world_seed(p, seed);
    }

    if (ocean_rnd != nullptr)
    {
        ocean_rnd_init(ocean_rnd, seed);
    }

    world_seed = seed;
    world_seed = algorithm::lcg_rand_next(world_seed);
    world_seed += base_seed;
    world_seed = algorithm::lcg_rand_next(world_seed);
    world_seed += base_seed;
    world_seed = algorithm::lcg_rand_next(world_seed);
    world_seed += base_seed;

    return world_seed;
}

int world_layer::mc_next_int(int mod)
{
    int ret = static_cast<int>((chunk_seed >> 24) % static_cast<int64_t>(mod));

    if (ret < 0)
    {
        ret += mod;
    }

    chunk_seed = algorithm::lcg_rand_next(chunk_seed);
    chunk_seed += world_seed;
    return ret;
}

void world_layer::map_island()
{
    int x, z;

    const int64_t ss = algorithm::lcg_rand_next(world_seed);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            const int64_t chunk_x = (int64_t)(x + area_x);
            const int64_t chunk_z = (int64_t)(z + area_z);
            int64_t cs = ss;
            cs += chunk_x;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_z;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_x;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_z;

            out[x + z * area_width] = (cs >> 24) % 10 == 0;
        }
    }

    if (area_x > -area_width && area_x <= 0 && area_z > -area_height && area_z <= 0)
    {
        out[-area_x + -area_z * area_width] = 1;
    }
}

void world_layer::map_zoom(Layer* l, int* __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int p_x = area_x >> 1;
    int p_z = area_z >> 1;
    int p_width = ((area_x + area_width) >> 1) - p_x + 1;
    int p_height = ((area_z + area_height) >> 1) - p_z + 1;
    int x, z;

    // printf("[%d %d] [%d %d]\n", p_x, p_z, p_width, p_height);
    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    int new_width = (p_width) << 1;
    int new_height = (p_height) << 1;
    int idx, a, b;

    std::vector<int> buf((new_width + 1) * (new_height + 1));

    const int ss = algorithm::lcg_rand_next(static_cast<int>(world_seed));

    for (z = 0; z < p_height; z++)
    {
        idx = (z << 1) * new_width;
        a = out[(z + 0) * p_width];
        b = out[(z + 1) * p_width];

        for (x = 0; x < p_width; x++)
        {
            int a1 = out[x + 1 + (z + 0) * p_width];
            int b1 = out[x + 1 + (z + 1) * p_width];

            const int chunk_x = (x + p_x) << 1;
            const int chunk_z = (z + p_z) << 1;

            int cs = ss;
            cs += chunk_x;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_z;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_x;
            cs = algorithm::lcg_rand_next(cs);
            cs += chunk_z;

            buf[idx] = a;
            buf[idx + new_width] = (cs >> 24) & 1 ? b : a;
            idx++;

            cs = algorithm::lcg_rand_next(cs);
            cs += ws;
            buf[idx] = (cs >> 24) & 1 ? a1 : a;

            if (parent1->get_map == world_layer::map_island)
            {
                // select_random4
                cs = algorithm::lcg_rand_next(cs);
                cs += ws;
                const int i = (cs >> 24) & 3;
                buf[idx + new_width] = i == 0 ? a : i == 1 ? a1 : i == 2 ? b : b1;
            }
            else
            {
                // select_mode_or_random
                if (a1 == b && b == b1)
                    buf[idx + new_width] = a1;
                else if (a == a1 && a == b)
                    buf[idx + new_width] = a;
                else if (a == a1 && a == b1)
                    buf[idx + new_width] = a;
                else if (a == b && a == b1)
                    buf[idx + new_width] = a;
                else if (a == a1 && b != b1)
                    buf[idx + new_width] = a;
                else if (a == b && a1 != b1)
                    buf[idx + new_width] = a;
                else if (a == b1 && a1 != b)
                    buf[idx + new_width] = a;
                else if (a1 == b && a != b1)
                    buf[idx + new_width] = a1;
                else if (a1 == b1 && a != b)
                    buf[idx + new_width] = a1;
                else if (b == b1 && a != a1)
                    buf[idx + new_width] = b;
                else
                {
                    cs = algorithm::lcg_rand_next(cs);
                    cs += ws;
                    const int i = (cs >> 24) & 3;
                    buf[idx + new_width] = i == 0 ? a : i == 1 ? a1 : i == 2 ? b : b1;
                }
            }

            idx++;
            a = a1;
            b = b1;
        }
    }

    for (z = 0; z < area_height; z++)
    {
        memcpy(&out[z * area_width], &buf[(z + (area_z & 1)) * new_width + (area_x & 1)], area_width * sizeof(int));
    }

    free(buf);
}

void world_layer::map_add_island()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    const int64_t ws = world_seed;
    const int64_t ss = ws * (ws * knuth_lcg_mmix_multiplier + knuth_lcg_mmix_increment);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v00 = out[x + 0 + (z + 0) * p_width];
            int v20 = out[x + 2 + (z + 0) * p_width];
            int v02 = out[x + 0 + (z + 2) * p_width];
            int v22 = out[x + 2 + (z + 2) * p_width];
            int v11 = out[x + 1 + (z + 1) * p_width];

            if (v11 == 0 && (v00 != 0 || v20 != 0 || v02 != 0 || v22 != 0))
            {
                const int64_t chunk_x = (int64_t)(x + area_x);
                const int64_t chunk_z = (int64_t)(z + area_z);
                int64_t cs = ss;
                cs += chunk_x;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_z;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_x;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_z;

                int v = 1;
                int inc = 0;

                if (v00 != 0)
                {
                    ++inc;
                    v = v00;
                    cs = algorithm::lcg_rand_next(cs);
                    cs += ws;
                }
                if (v20 != 0)
                {
                    if (++inc == 1 || (cs & (1_lL << 24)) == 0)
                        v = v20;
                    cs = algorithm::lcg_rand_next(cs);
                    cs += ws;
                }
                if (v02 != 0)
                {
                    switch (++inc)
                    {
                        case 1:
                            v = v02;
                            break;
                        case 2:
                            if ((cs & (1_lL << 24)) == 0)
                                v = v02;
                            break;
                        default:
                            if (((cs >> 24) % 3) == 0)
                                v = v02;
                    }
                    cs = algorithm::lcg_rand_next(cs);
                    cs += ws;
                }
                if (v22 != 0)
                {
                    switch (++inc)
                    {
                        case 1:
                            v = v22;
                            break;
                        case 2:
                            if ((cs & (1_lL << 24)) == 0)
                                v = v22;
                            break;
                        case 3:
                            if (((cs >> 24) % 3) == 0)
                                v = v22;
                            break;
                        default:
                            if ((cs & (3_lL << 24)) == 0)
                                v = v22;
                    }
                    cs = algorithm::lcg_rand_next(cs);
                    cs += ws;
                }

                if ((cs >> 24) % 3 == 0)
                    out[x + z * area_width] = v;
                else if (v == 4)
                    out[x + z * area_width] = 4;
                else
                    out[x + z * area_width] = 0;
            }
            else if (v11 > 0 && (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0))
            {
                // set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
                // if (mc_next_int(l, 5) == 0)...

                const int64_t chunk_x = (int64_t)(x + area_x);
                const int64_t chunk_z = (int64_t)(z + area_z);

                int64_t cs = ss;
                cs += chunk_x;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_z;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_x;
                cs = algorithm::lcg_rand_next(cs);
                cs += chunk_z;

                if ((cs >> 24) % 5 == 0)
                    out[x + z * area_width] = (v11 == 4) ? 4 : 0;
                else
                    out[x + z * area_width] = v11;
            }
            else
            {
                out[x + z * area_width] = v11;
            }
        }
    }
}

void world_layer::map_remove_too_much_ocean()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];
            out[x + z * area_width] = v11;

            if (out[x + 1 + (z + 0) * p_width] != 0)
                continue;
            if (out[x + 2 + (z + 1) * p_width] != 0)
                continue;
            if (out[x + 0 + (z + 1) * p_width] != 0)
                continue;
            if (out[x + 1 + (z + 2) * p_width] != 0)
                continue;

            if (v11 == 0)
            {
                set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));

                if (mc_next_int(l, 2) == 0)
                {
                    out[x + z * area_width] = 1;
                }
            }
        }
    }
}

void world_layer::map_add_snow()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];

            if (is_shallow_ocean(v11))
            {
                out[x + z * area_width] = v11;
            }
            else
            {
                set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
                int r = mc_next_int(l, 6);
                int v;

                if (r == 0)
                    v = 4;
                else if (r <= 1)
                    v = 3;
                else
                    v = 1;

                out[x + z * area_width] = v;
            }
        }
    }
}

void world_layer::map_cool_warm()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];

            if (v11 == 1)
            {
                int v10 = out[x + 1 + (z + 0) * p_width];
                int v21 = out[x + 2 + (z + 1) * p_width];
                int v01 = out[x + 0 + (z + 1) * p_width];
                int v12 = out[x + 1 + (z + 2) * p_width];

                if (v10 == 3 || v10 == 4 || v21 == 3 || v21 == 4 || v01 == 3 || v01 == 4 || v12 == 3 || v12 == 4)
                {
                    v11 = 2;
                }
            }

            out[x + z * area_width] = v11;
        }
    }
}

void world_layer::map_heat_ice()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];

            if (v11 == 4)
            {
                int v10 = out[x + 1 + (z + 0) * p_width];
                int v21 = out[x + 2 + (z + 1) * p_width];
                int v01 = out[x + 0 + (z + 1) * p_width];
                int v12 = out[x + 1 + (z + 2) * p_width];

                if (v10 == 1 || v10 == 2 || v21 == 1 || v21 == 2 || v01 == 1 || v01 == 2 || v12 == 1 || v12 == 2)
                {
                    v11 = 3;
                }
            }

            out[x + z * area_width] = v11;
        }
    }
}

void world_layer::map_special()
{
    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height);

    int x, z;
    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v = out[x + z * area_width];
            if (v == 0)
                continue;

            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));

            if (mc_next_int(l, 13) == 0)
            {
                v |= (1 + mc_next_int(l, 15)) << 8 & 0xf00;
                // 1 to 1 mapping so 'out' can be overwritten immediately
                out[x + z * area_width] = v;
            }
        }
    }
}

void world_layer::map_add_mushroom_island()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];

            // surrounded by ocean?
            if (v11 == 0 && !out[x + 0 + (z + 0) * p_width] && !out[x + 2 + (z + 0) * p_width]
                && !out[x + 0 + (z + 2) * p_width] && !out[x + 2 + (z + 2) * p_width])
            {
                set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
                if (mc_next_int(l, 100) == 0)
                {
                    out[x + z * area_width] = mushroom_fields;
                    continue;
                }
            }

            out[x + z * area_width] = v11;
        }
    }
}

void world_layer::map_deep_ocean()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[(x + 1) + (z + 1) * p_width];

            if (is_shallow_ocean(v11))
            {
                // count adjacent oceans
                int oceans = 0;
                if (is_shallow_ocean(out[(x + 1) + (z + 0) * p_width]))
                    oceans++;
                if (is_shallow_ocean(out[(x + 2) + (z + 1) * p_width]))
                    oceans++;
                if (is_shallow_ocean(out[(x + 0) + (z + 1) * p_width]))
                    oceans++;
                if (is_shallow_ocean(out[(x + 1) + (z + 2) * p_width]))
                    oceans++;

                if (oceans > 3)
                {
                    switch (v11)
                    {
                        case warm_ocean:
                            v11 = deep_warm_ocean;
                            break;
                        case lukewarm_ocean:
                            v11 = deep_lukewarm_ocean;
                            break;
                        case ocean:
                            v11 = deep_ocean;
                            break;
                        case cold_ocean:
                            v11 = deep_cold_ocean;
                            break;
                        case frozen_ocean:
                            v11 = deep_frozen_ocean;
                            break;
                        default:
                            v11 = deep_ocean;
                    }
                }
            }

            out[x + z * area_width] = v11;
        }
    }
}

const int warm_biomes[] = {desert, desert, desert, savanna, savanna, plains};
const int lush_biomes[] = {forest, dark_forest, mountains, plains, birch_forest, swamp};
const int cold_biomes[] = {forest, mountains, taiga, plains};
const int snow_biomes[] = {snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga};

void world_layer::map_biome()
{
    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height);

    int x, z;
    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int idx = x + z * area_width;
            int id = out[idx];
            int has_high_bit = (id & 0xf00) >> 8;
            id &= -0xf01;

            if (get_biome_type(id) == Ocean || id == mushroom_fields)
            {
                out[idx] = id;
                continue;
            }

            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));

            switch (id)
            {
                case Warm:
                    if (has_high_bit)
                        out[idx] = (mc_next_int(l, 3) == 0) ? badlands_plateau : wooded_badlands_plateau;
                    else
                        out[idx] = warm_biomes[mc_next_int(l, 6)];
                    break;
                case Lush:
                    if (has_high_bit)
                        out[idx] = jungle;
                    else
                        out[idx] = lush_biomes[mc_next_int(l, 6)];
                    break;
                case Cold:
                    if (has_high_bit)
                        out[idx] = giant_tree_taiga;
                    else
                        out[idx] = cold_biomes[mc_next_int(l, 4)];
                    break;
                case Freezing:
                    out[idx] = snow_biomes[mc_next_int(l, 4)];
                    break;
                default:
                    out[idx] = mushroom_fields;
            }
        }
    }
}

void world_layer::map_biome_be()
{
    static constexpr int lush_biomes_be[] = {
        forest, dark_forest, mountains, plains, plains, plains, birch_forest, swamp};

    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height);

    int x, z;
    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int idx = x + z * area_width;
            int id = out[idx];
            int has_high_bit = (id & 0xf00) >> 8;
            id &= -0xf01;

            if (get_biome_type(id) == Ocean || id == mushroom_fields)
            {
                out[idx] = id;
                continue;
            }

            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));

            switch (id)
            {
                case Warm:
                    if (has_high_bit)
                        out[idx] = (mc_next_int(l, 3) == 0) ? badlands_plateau : wooded_badlands_plateau;
                    else
                        out[idx] = warm_biomes[mc_next_int(l, 6)];
                    break;
                case Lush:
                    if (has_high_bit)
                        out[idx] = jungle;
                    else
                        out[idx] = lush_biomes_be[mc_next_int(l, 6)];
                    break;
                case Cold:
                    if (has_high_bit)
                        out[idx] = giant_tree_taiga;
                    else
                        out[idx] = cold_biomes[mc_next_int(l, 4)];
                    break;
                case Freezing:
                    out[idx] = snow_biomes[mc_next_int(l, 4)];
                    break;
                default:
                    out[idx] = mushroom_fields;
            }
        }
    }
}

void world_layer::map_river_init()
{
    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height);

    int x, z;
    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            if (out[x + z * area_width] > 0)
            {
                set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
                out[x + z * area_width] = mc_next_int(l, 299999) + 2;
            }
            else
            {
                out[x + z * area_width] = 0;
            }
        }
    }
}

void world_layer::map_add_bamboo()
{
    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height);

    int x, z;
    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int idx = x + z * area_width;
            if (out[idx] != jungle)
                continue;

            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
            if (mc_next_int(l, 10) == 0)
            {
                out[idx] = bamboo_jungle;
            }
        }
    }
}

static inline int replace_edge(int* out, int idx, int v10, int v21, int v01, int v12, int id, int base_iD, int edge_iD)
{
    if (id != base_iD)
        return 0;

    // are_similar() has not changed behaviour for ids < MUTATED_BIOME_OFFSET, so use the faster
    // variant
    if (are_similar113(v10, base_iD) && are_similar113(v21, base_iD) && are_similar113(v01, base_iD)
        && are_similar113(v12, base_iD))
        out[idx] = id;
    else
        out[idx] = edge_iD;

    return 1;
}

void world_layer::map_biome_edge()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];

            int v10 = out[x + 1 + (z + 0) * p_width];
            int v21 = out[x + 2 + (z + 1) * p_width];
            int v01 = out[x + 0 + (z + 1) * p_width];
            int v12 = out[x + 1 + (z + 2) * p_width];

            if (!replace_edge(out, x + z * area_width, v10, v21, v01, v12, v11, wooded_badlands_plateau, badlands)
                && !replace_edge(out, x + z * area_width, v10, v21, v01, v12, v11, badlands_plateau, badlands)
                && !replace_edge(out, x + z * area_width, v10, v21, v01, v12, v11, giant_tree_taiga, taiga))
            {
                if (v11 == desert)
                {
                    if (v10 != snowy_tundra && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
                    {
                        out[x + z * area_width] = v11;
                    }
                    else
                    {
                        out[x + z * area_width] = wooded_mountains;
                    }
                }
                else if (v11 == swamp)
                {
                    if (v10 != desert && v21 != desert && v01 != desert && v12 != desert && v10 != snowy_taiga
                        && v21 != snowy_taiga && v01 != snowy_taiga && v12 != snowy_taiga && v10 != snowy_tundra
                        && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
                    {
                        if (v10 != jungle && v12 != jungle && v21 != jungle && v01 != jungle && v10 != bamboo_jungle
                            && v12 != bamboo_jungle && v21 != bamboo_jungle && v01 != bamboo_jungle)
                            out[x + z * area_width] = v11;
                        else
                            out[x + z * area_width] = jungle_edge;
                    }
                    else
                    {
                        out[x + z * area_width] = plains;
                    }
                }
                else
                {
                    out[x + z * area_width] = v11;
                }
            }
        }
    }
}

void world_layer::map_hills()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;
    int* buf = nullptr;

    if (parent2 == nullptr)
    {
        printf("world_layer::map_hills() requires two parents! Use setup_multi_layer()\n");
        exit(1);
    }

    buf = ( int* ) malloc(p_width * p_height * sizeof(int));

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);
    memcpy(buf, out, p_width * p_height * sizeof(int));

    parent2->get_map(parent2, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
            int a11 = buf[x + 1 + (z + 1) * p_width]; // biome branch
            int b11 = out[x + 1 + (z + 1) * p_width]; // river branch
            int idx = x + z * area_width;

            int var12 = (b11 - 2) % 29 == 0;

            if (a11 != 0 && b11 >= 2 && (b11 - 2) % 29 == 1 && a11 < MUTATED_BIOME_OFFSET)
            {
                out[idx] = (biome_exists(a11 + MUTATED_BIOME_OFFSET)) ? a11 + MUTATED_BIOME_OFFSET : a11;
            }
            else if (mc_next_int(l, 3) != 0 && !var12)
            {
                out[idx] = a11;
            }
            else
            {
                int hill_iD = a11;

                switch (a11)
                {
                    case desert:
                        hill_iD = desert_hills;
                        break;
                    case forest:
                        hill_iD = wooded_hills;
                        break;
                    case birch_forest:
                        hill_iD = birch_forest_hills;
                        break;
                    case dark_forest:
                        hill_iD = plains;
                        break;
                    case taiga:
                        hill_iD = taiga_hills;
                        break;
                    case giant_tree_taiga:
                        hill_iD = giant_tree_taiga_hills;
                        break;
                    case snowy_taiga:
                        hill_iD = snowy_taiga_hills;
                        break;
                    case plains:
                        hill_iD = (mc_next_int(l, 3) == 0) ? wooded_hills : forest;
                        break;
                    case snowy_tundra:
                        hill_iD = snowy_mountains;
                        break;
                    case jungle:
                        hill_iD = jungle_hills;
                        break;
                    case ocean:
                        hill_iD = deep_ocean;
                        break;
                    case mountains:
                        hill_iD = wooded_mountains;
                        break;
                    case savanna:
                        hill_iD = savanna_plateau;
                        break;
                    default:
                        if (are_similar(a11, wooded_badlands_plateau))
                            hill_iD = badlands;
                        else if (a11 == deep_ocean && mc_next_int(l, 3) == 0)
                            hill_iD = (mc_next_int(l, 2) == 0) ? plains : forest;
                        break;
                }

                if (var12 && hill_iD != a11)
                {
                    if (biome_exists(hill_iD + MUTATED_BIOME_OFFSET))
                        hill_iD += MUTATED_BIOME_OFFSET;
                    else
                        hill_iD = a11;
                }

                if (hill_iD == a11)
                {
                    out[idx] = a11;
                }
                else
                {
                    int a10 = buf[x + 1 + (z + 0) * p_width];
                    int a21 = buf[x + 2 + (z + 1) * p_width];
                    int a01 = buf[x + 0 + (z + 1) * p_width];
                    int a12 = buf[x + 1 + (z + 2) * p_width];
                    int equals = 0;

                    if (are_similar(a10, a11))
                        equals++;
                    if (are_similar(a21, a11))
                        equals++;
                    if (are_similar(a01, a11))
                        equals++;
                    if (are_similar(a12, a11))
                        equals++;

                    if (equals >= 3)
                        out[idx] = hill_iD;
                    else
                        out[idx] = a11;
                }
            }
        }
    }

    free(buf);
}

static inline int reduce_iD(int id)
{
    return id >= 2 ? 2 + (id & 1) : id;
}

void world_layer::map_river()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v01 = reduce_iD(out[x + 0 + (z + 1) * p_width]);
            int v21 = reduce_iD(out[x + 2 + (z + 1) * p_width]);
            int v10 = reduce_iD(out[x + 1 + (z + 0) * p_width]);
            int v12 = reduce_iD(out[x + 1 + (z + 2) * p_width]);
            int v11 = reduce_iD(out[x + 1 + (z + 1) * p_width]);

            if (v11 == v01 && v11 == v10 && v11 == v21 && v11 == v12)
            {
                out[x + z * area_width] = -1;
            }
            else
            {
                out[x + z * area_width] = river;
            }
        }
    }
}

void world_layer::map_smooth()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];
            int v10 = out[x + 1 + (z + 0) * p_width];
            int v21 = out[x + 2 + (z + 1) * p_width];
            int v01 = out[x + 0 + (z + 1) * p_width];
            int v12 = out[x + 1 + (z + 2) * p_width];

            if (v01 == v21 && v10 == v12)
            {
                set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));

                if (mc_next_int(l, 2) == 0)
                    v11 = v01;
                else
                    v11 = v10;
            }
            else
            {
                if (v01 == v21)
                    v11 = v01;
                if (v10 == v12)
                    v11 = v10;
            }

            out[x + z * area_width] = v11;
        }
    }
}

void world_layer::map_rare_biome()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            set_chunk_seed(l, (int64_t)(x + area_x), (int64_t)(z + area_z));
            int v11 = out[x + 1 + (z + 1) * p_width];

            if (mc_next_int(l, 57) == 0 && v11 == plains)
            {
                // Sunflower Plains
                out[x + z * area_width] = plains + MUTATED_BIOME_OFFSET;
            }
            else
            {
                out[x + z * area_width] = v11;
            }
        }
    }
}

inline static int replace_ocean(int* out, int idx, int v10, int v21, int v01, int v12, int id, int replace_iD)
{
    if (is_oceanic(id))
        return 0;

    if (!is_oceanic(v10) && !is_oceanic(v21) && !is_oceanic(v01) && !is_oceanic(v12))
        out[idx] = id;
    else
        out[idx] = replace_iD;

    return 1;
}

inline static int is_biome_jFTO(int id)
{
    return biome_exists(id) && (get_biome_type(id) == Jungle || id == forest || id == taiga || is_oceanic(id));
}

void world_layer::map_shore()
{
    int p_x = area_x - 1;
    int p_z = area_z - 1;
    int p_width = area_width + 2;
    int p_height = area_height + 2;
    int x, z;

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int v11 = out[x + 1 + (z + 1) * p_width];
            int v10 = out[x + 1 + (z + 0) * p_width];
            int v21 = out[x + 2 + (z + 1) * p_width];
            int v01 = out[x + 0 + (z + 1) * p_width];
            int v12 = out[x + 1 + (z + 2) * p_width];

            int biome = biome_exists(v11) ? v11 : 0;

            if (v11 == mushroom_fields)
            {
                if (v10 != ocean && v21 != ocean && v01 != ocean && v12 != ocean)
                    out[x + z * area_width] = v11;
                else
                    out[x + z * area_width] = mushroom_field_shore;
            }
            else if (/*biome < MUTATED_BIOME_OFFSET &&*/ get_biome_type(biome) == Jungle)
            {
                if (is_biome_jFTO(v10) && is_biome_jFTO(v21) && is_biome_jFTO(v01) && is_biome_jFTO(v12))
                {
                    if (!is_oceanic(v10) && !is_oceanic(v21) && !is_oceanic(v01) && !is_oceanic(v12))
                        out[x + z * area_width] = v11;
                    else
                        out[x + z * area_width] = beach;
                }
                else
                {
                    out[x + z * area_width] = jungle_edge;
                }
            }
            else if (v11 != mountains && v11 != wooded_mountains && v11 != mountain_edge)
            {
                if (is_biome_snowy(biome))
                {
                    replace_ocean(out, x + z * area_width, v10, v21, v01, v12, v11, snowy_beach);
                }
                else if (v11 != badlands && v11 != wooded_badlands_plateau)
                {
                    if (v11 != ocean && v11 != deep_ocean && v11 != river && v11 != swamp)
                    {
                        if (!is_oceanic(v10) && !is_oceanic(v21) && !is_oceanic(v01) && !is_oceanic(v12))
                            out[x + z * area_width] = v11;
                        else
                            out[x + z * area_width] = beach;
                    }
                    else
                    {
                        out[x + z * area_width] = v11;
                    }
                }
                else
                {
                    if (!is_oceanic(v10) && !is_oceanic(v21) && !is_oceanic(v01) && !is_oceanic(v12))
                    {
                        if (get_biome_type(v10) == Mesa && get_biome_type(v21) == Mesa && get_biome_type(v01) == Mesa
                            && get_biome_type(v12) == Mesa)
                            out[x + z * area_width] = v11;
                        else
                            out[x + z * area_width] = desert;
                    }
                    else
                    {
                        out[x + z * area_width] = v11;
                    }
                }
            }
            else
            {
                replace_ocean(out, x + z * area_width, v10, v21, v01, v12, v11, stone_shore);
            }
        }
    }
}

void world_layer::map_river_mix()
{
    int idx;
    int len;
    int* buf;

    if (parent2 == nullptr)
    {
        printf("world_layer::map_river_mix() requires two parents! Use setup_multi_layer()\n");
        exit(1);
    }

    len = area_width * area_height;
    buf = ( int* ) malloc(len * sizeof(int));

    parent1->get_map(parent1, out, area_x, area_z, area_width, area_height); // biome chain
    memcpy(buf, out, len * sizeof(int));

    parent2->get_map(parent2, out, area_x, area_z, area_width, area_height); // rivers

    for (idx = 0; idx < len; idx++)
    {
        if (is_oceanic(buf[idx]))
        {
            out[idx] = buf[idx];
        }
        else
        {
            if (out[idx] == river)
            {
                if (buf[idx] == snowy_tundra)
                    out[idx] = frozen_river;
                else if (buf[idx] == mushroom_fields || buf[idx] == mushroom_field_shore)
                    out[idx] = mushroom_field_shore;
                else
                    out[idx] = out[idx] & 255;
            }
            else
            {
                out[idx] = buf[idx];
            }
        }
    }

    free(buf);
}

/*
 * Initialises data for the ocean temperature types using the world seed.
 */
static void ocean_rnd_init(ocean_rnd* rnd, int64_t seed)
{
    int i = 0;
    memset(rnd, 0, sizeof(*rnd));
    set_seed(&seed);
    rnd->a = next_double(&seed) * 256.0;
    rnd->b = next_double(&seed) * 256.0;
    rnd->c = next_double(&seed) * 256.0;

    for (i = 0; i < 256; i++)
    {
        rnd->d[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        int n3 = next_int(&seed, 256 - i) + i;
        int n4 = rnd->d[i];
        rnd->d[i] = rnd->d[n3];
        rnd->d[n3] = n4;
        rnd->d[i + 256] = rnd->d[i];
    }
}

static double world_layer::get_ocean_temp(const ocean_rnd* rnd, double d1, double d2, double d3)
{
    /* Table of vectors to cube edge centres (12 + 4 extra), used for ocean PRNG */
    static constexpr double c_edge_x[] = {
        1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0};
    static constexpr double c_edge_y[] = {
        1.0, 1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0};
    static constexpr double c_edge_z[] = {
        0.0, 0.0, 0.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0, -1.0};

    static constexpr auto indexed_lerp = [](int idx, const double d1, const double d2, const double d3) {
        idx &= 0xf;
        return c_edge_x[idx] * d1 + c_edge_y[idx] * d2 + c_edge_z[idx] * d3;
    };

    static constexpr auto lerp
        = [](const double part, const double from, const double to) -> double { return from + part * (to - from); };

    d1 += rnd->a;
    d2 += rnd->b;
    d3 += rnd->c;
    int i1 = ( int ) d1 - ( int ) (d1 < 0);
    int i2 = ( int ) d2 - ( int ) (d2 < 0);
    int i3 = ( int ) d3 - ( int ) (d3 < 0);
    d1 -= i1;
    d2 -= i2;
    d3 -= i3;
    double t1 = d1 * d1 * d1 * (d1 * (d1 * 6.0 - 15.0) + 10.0);
    double t2 = d2 * d2 * d2 * (d2 * (d2 * 6.0 - 15.0) + 10.0);
    double t3 = d3 * d3 * d3 * (d3 * (d3 * 6.0 - 15.0) + 10.0);

    i1 &= 0xff;
    i2 &= 0xff;
    i3 &= 0xff;

    int a1 = rnd->d[i1] + i2;
    int a2 = rnd->d[a1] + i3;
    int a3 = rnd->d[a1 + 1] + i3;
    int b1 = rnd->d[i1 + 1] + i2;
    int b2 = rnd->d[b1] + i3;
    int b3 = rnd->d[b1 + 1] + i3;

    double l1 = indexed_lerp(rnd->d[a2], d1, d2, d3);
    double l2 = indexed_lerp(rnd->d[b2], d1 - 1, d2, d3);
    double l3 = indexed_lerp(rnd->d[a3], d1, d2 - 1, d3);
    double l4 = indexed_lerp(rnd->d[b3], d1 - 1, d2 - 1, d3);
    double l5 = indexed_lerp(rnd->d[a2 + 1], d1, d2, d3 - 1);
    double l6 = indexed_lerp(rnd->d[b2 + 1], d1 - 1, d2, d3 - 1);
    double l7 = indexed_lerp(rnd->d[a3 + 1], d1, d2 - 1, d3 - 1);
    double l8 = indexed_lerp(rnd->d[b3 + 1], d1 - 1, d2 - 1, d3 - 1);

    l1 = lerp(t1, l1, l2);
    l3 = lerp(t1, l3, l4);
    l5 = lerp(t1, l5, l6);
    l7 = lerp(t1, l7, l8);

    l1 = lerp(t2, l1, l3);
    l5 = lerp(t2, l5, l7);

    return lerp(t3, l1, l5);
}

void world_layer::map_ocean_temp()
{
    int x, z;

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            double tmp = get_ocean_temp(ocean_rnd, (x + area_x) / 8.0, (z + area_z) / 8.0, 0);

            if (tmp > 0.4)
                out[x + z * area_width] = warm_ocean;
            else if (tmp > 0.2)
                out[x + z * area_width] = lukewarm_ocean;
            else if (tmp < -0.4)
                out[x + z * area_width] = frozen_ocean;
            else if (tmp < -0.2)
                out[x + z * area_width] = cold_ocean;
            else
                out[x + z * area_width] = ocean;
        }
    }
}

/* Warning: this function is horribly slow compared to other layers! */
void world_layer::map_ocean_mix()
{
    int land_x = area_x - 8, land_z = area_z - 8;
    int land_width = area_width + 17, land_height = area_height + 17;
    int *map1, *map2;

    if (parent2 == nullptr)
    {
        printf("world_layer::map_ocean_mix() requires two parents! Use setup_multi_layer()\n");
        exit(1);
    }

    parent1->get_map(parent1, out, land_x, land_z, land_width, land_height);
    map1 = ( int* ) malloc(land_width * land_height * sizeof(int));
    memcpy(map1, out, land_width * land_height * sizeof(int));

    parent2->get_map(parent2, out, area_x, area_z, area_width, area_height);
    map2 = ( int* ) malloc(area_width * area_height * sizeof(int));
    memcpy(map2, out, area_width * area_height * sizeof(int));

    int x, z, i, j;

    for (z = 0; z < area_height; z++)
    {
        for (x = 0; x < area_width; x++)
        {
            int land_iD = map1[(x + 8) + (z + 8) * land_width];
            int ocean_iD = map2[x + z * area_width];

            if (!is_oceanic(land_iD))
            {
                out[x + z * area_width] = land_iD;
                continue;
            }

            for (i = -8; i <= 8; i += 4)
            {
                for (j = -8; j <= 8; j += 4)
                {
                    int nearby_iD = map1[(x + i + 8) + (z + j + 8) * land_width];

                    if (is_oceanic(nearby_iD))
                        continue;

                    if (ocean_iD == warm_ocean)
                    {
                        out[x + z * area_width] = lukewarm_ocean;
                        goto loop_x;
                    }

                    if (ocean_iD == frozen_ocean)
                    {
                        out[x + z * area_width] = cold_ocean;
                        goto loop_x;
                    }
                }
            }

            if (land_iD == deep_ocean)
            {
                switch (ocean_iD)
                {
                    case lukewarm_ocean:
                        ocean_iD = deep_lukewarm_ocean;
                        break;
                    case ocean:
                        ocean_iD = deep_ocean;
                        break;
                    case cold_ocean:
                        ocean_iD = deep_cold_ocean;
                        break;
                    case frozen_ocean:
                        ocean_iD = deep_frozen_ocean;
                        break;
                }
            }

            out[x + z * area_width] = ocean_iD;

        loop_x:;
        }
    }

    free(map1);
    free(map2);
}

void world_layer::map_voronoi_zoom()
{
    area_x -= 2;
    area_z -= 2;
    int p_x = area_x >> 2;
    int p_z = area_z >> 2;
    int p_width = (area_width >> 2) + 2;
    int p_height = (area_height >> 2) + 2;
    int new_width = (p_width - 1) << 2;
    int new_height = (p_height - 1) << 2;
    int x, z, i, j;
    int* buf = ( int* ) malloc((new_width + 1) * (new_height + 1) * sizeof(*buf));

    parent1->get_map(parent1, out, p_x, p_z, p_width, p_height);

    for (z = 0; z < p_height - 1; z++)
    {
        int v00 = out[(z + 0) * p_width];
        int v01 = out[(z + 1) * p_width];

        for (x = 0; x < p_width - 1; x++)
        {
            set_chunk_seed(l, (x + p_x) << 2, (z + p_z) << 2);
            double da1 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6;
            double da2 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6;

            set_chunk_seed(l, (x + p_x + 1) << 2, (z + p_z) << 2);
            double db1 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double db2 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6;

            set_chunk_seed(l, (x + p_x) << 2, (z + p_z + 1) << 2);
            double dc1 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6;
            double dc2 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            set_chunk_seed(l, (x + p_x + 1) << 2, (z + p_z + 1) << 2);
            double dd1 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double dd2 = (mc_next_int(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            int v10 = out[x + 1 + (z + 0) * p_width] & 255;
            int v11 = out[x + 1 + (z + 1) * p_width] & 255;

            for (j = 0; j < 4; j++)
            {
                int idx = ((z << 2) + j) * new_width + (x << 2);

                for (i = 0; i < 4; i++)
                {
                    double da = (j - da2) * (j - da2) + (i - da1) * (i - da1);
                    double db = (j - db2) * (j - db2) + (i - db1) * (i - db1);
                    double dc = (j - dc2) * (j - dc2) + (i - dc1) * (i - dc1);
                    double dd = (j - dd2) * (j - dd2) + (i - dd1) * (i - dd1);

                    if (da < db && da < dc && da < dd)
                    {
                        buf[idx++] = v00;
                    }
                    else if (db < da && db < dc && db < dd)
                    {
                        buf[idx++] = v10;
                    }
                    else if (dc < da && dc < db && dc < dd)
                    {
                        buf[idx++] = v01;
                    }
                    else
                    {
                        buf[idx++] = v11;
                    }
                }
            }

            v00 = v10;
            v01 = v11;
        }
    }

    for (z = 0; z < area_height; z++)
    {
        memcpy(&out[z * area_width], &buf[(z + (area_z & 3)) * new_width + (area_x & 3)], area_width * sizeof(int));
    }

    free(buf);
}

} // namespace minecraft::biome_generator
