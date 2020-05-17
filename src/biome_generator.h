#include <limits>
#include <memory>

#include "algorithm.h"
#include "java_random.h"

namespace minecraft::biome_generator
{
struct world_area
{
    int x;
    int z;
    int width;
    int height;
};

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  layer                                                                    //
//  Detailed description                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

class world_layer
{
public:
    world_layer() noexcept = delete;
    ~world_layer() noexcept = default;
    world_layer(const world_layer& other) noexcept = default;
    world_layer& operator=(const world_layer& other) noexcept = default;
    world_layer(world_layer&& other) noexcept = default;
    world_layer& operator=(world_layer&& other) noexcept = default;

    world_layer(int scale_, world_layer* parent1_, world_layer* parent2_, int seed);

    void map_null();
    void map_skip();
    void map_island();
    void map_zoom();
    void map_add_island();
    void map_remove_too_much_ocean();
    void map_add_snow();
    void map_cool_warm();
    void map_heat_ice();
    void map_special();
    void map_add_mushroom_island();
    void map_deep_ocean();
    void map_biome();
    void map_biome_be();
    void map_river_init();
    void map_add_bamboo();
    void map_biome_edge();
    void map_hills();
    void map_hills113();
    void map_river();
    void map_smooth();
    void map_rare_biome();
    void map_shore();
    void map_river_mix();
    void map_ocean_temp();
    void map_ocean_mix();
    void map_voronoi_zoom();

private:
    struct ocean_rnd
    {
        int d[512];
        double a, b, c;
    };

    int64_t base_seed; // generator seed (depends only on layer hierarchy)
    int64_t world_seed; // based on the seed of the world
    ocean_rnd* layer_ocean_rnd; // world seed dependent data for ocean temperatures
    int scale; // map scale of this layer (map entry = scale x scale blocks)
    void (*getmap)(world_layer* layer, int* out, int x, int z, int w, int h);
    world_area area;

    std::shared_ptr<world_layer> parent1;
    std::shared_ptr<world_layer> parent2;

    int mc_next_int(int mod);
    int64_t get_chunk_seed(int64_t chunk_x, int64_t chunk_z);
    int64_t get_base_seed(int64_t seed);
    int64_t get_world_seed(int64_t seed);
    static double get_ocean_temp(const ocean_rnd* rnd, double d1, double d2, double d3);
}; // class world_layer

} // namespace minecraft::biome_generator
