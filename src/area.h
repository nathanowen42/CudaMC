#include <cstdint>
#include <vector>

struct area_spec
{
    const int x_start;
    const int z_start;
    const size_t width;
    const size_t height;
}; // struct area

struct area_vector
{
    area_vector() = delete;
    ~area_vector() noexcept = default;
    area_vector(const area_vector& other) noexcept = default;
    area_vector& operator=(const area_vector& other) = delete;
    area_vector(area_vector&& other) noexcept = default;
    area_vector& operator=(area_vector&& other) = delete;

    area_vector(area);

    bool get_sub_area_vector(area_vector& out);

    area_spec area;

    std::vector<int> data;

}; // class area_vector

area::area(int x_start_, int z_start_, size_t width_, size_t height_)
    : x_start{x_start_}, z_start{z_start_}, width{width_}, height{height_}
{
    data.reserve(width * height);
}

bool area::get_sub_area(area& out)
{
    if (out.x_start < x_start || out.z_start < z_start || (out.x_start + out.width) > (x_start + width)
        || (out.z_start + out.height) > (z_start + height))
    {
        return false;
    }

    if (out.x_start == x_start && out.z_start == z_start && out.width == width && out.height == height)
    {
        out.data = data;
        return true;
    }

    auto x_offset = x_start - out.x_start;
    auto z_offset = z_start - out.z_start;

    for (size_t j = z_offset; j < (out.z_start + out.height); j++)
    {
        auto in_start = j * width;
        auto out_start = (j - z_offset) * out.width;
        std::copy(&data[in_start], &data[in_start + out.width], &out.data[out_start]);
    }

    return true;
}
