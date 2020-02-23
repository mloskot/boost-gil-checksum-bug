#include <boost/crc.hpp>
#include <boost/gil.hpp>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
namespace fs = std::filesystem;
namespace gil = boost::gil;

// TOOLS ///////////////////////////////////////////////////////////////////////
using bgr121_ref_t = gil::bit_aligned_pixel_reference
<
    std::uint8_t,
    boost::mp11::mp_list_c<int, 1, 2, 1>,
    gil::bgr_layout_t,
    true
> const;
using bgr121_image_t = gil::image<bgr121_ref_t, false>;
using bgr121_view_t = typename bgr121_image_t::view_t;
using bgr121_value_t = typename bgr121_view_t::value_type;

bgr121_value_t bgr121_red(0), bgr121_blue(0);

void init()
{
    gil::rgb8_pixel_t red8(255, 0, 0);
    gil::rgb8_pixel_t blue8(0, 0, 255);
    gil::color_convert(red8, bgr121_red);
    gil::color_convert(blue8, bgr121_blue);
}

template <typename View>
auto checksum(View const& img_view) -> std::string
{
    boost::crc_32_type crc;
    {
        gil::rgb8_image_t rgb_img(img_view.dimensions());
        gil::copy_and_convert_pixels(img_view, gil::view(rgb_img));
        auto rgb_view = gil::view(rgb_img);
        crc.process_bytes(rgb_view.row_begin(0), rgb_view.size() * 3);
    }
    std::ostringstream oss;
    oss << std::hex << crc.checksum();
    return oss.str();
}

template <typename View>
void save_dump(View const& view, std::string name)
{
    char const* const file_suffix =
#ifdef NDEBUG
        "_opt"
#else
        "_dbg"
#endif
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__))
        "_x64"
#else
        "_x32"
#endif
#ifdef BOOST_CLANG
        "_clang"
#endif
#ifdef BOOST_GCC
        "_gcc"
#endif
#ifdef BOOST_MSVC
        "_msvc"
#endif
        ;
    auto const filename = name + "_" + std::to_string(view.width()) + "x" + std::to_string(view.height());
    auto const dir_path = fs::path(__FILE__).parent_path(); // MSVC: requires -FC
    auto out_path = dir_path / (filename + file_suffix + ".txt");

    std::vector<std::tuple<int, int, int>> dump;
    dump.reserve(view.width() * view.height());
    gil::for_each_pixel(view, [&dump](auto const& p)
    {
        auto b = (int)get_color(p, gil::blue_t{});
        auto g = (int)get_color(p, gil::green_t{});
        auto r = (int)get_color(p, gil::red_t{});
        dump.emplace_back(r, g, b);
    });

    std::cout << out_path.string() << std::endl;
    std::ofstream ofs(out_path);
    ofs << checksum(view) << std::endl;
    for (auto& p : dump)
        ofs << std::get<0>(p) << '\t' << std::get<1>(p) << '\t' << std::get<1>(p) << "\n";
}

// TESTS ///////////////////////////////////////////////////////////////////////
// 1. Fill image with red.
// 2. Draw a blue line along the diagonal.
// 3. Calculate checksum.
void draw_loop(std::ptrdiff_t w, std::ptrdiff_t h)
{
    init();
    bgr121_image_t img(w, h);
    {
        auto v = view(img);
        fill(v.begin(), v.end(), bgr121_red);
        save_dump(view(img), "dump1");
    }
    {
        auto v = view(img);
        auto loc = v.xy_at(0, v.height() - 1);
        for (std::ptrdiff_t y = 0; y < v.height(); ++y)
        {
            *loc = bgr121_blue;
            ++loc.x();
            --loc.y();
        }
        save_dump(view(img), "dump2_loop");
    }
}

void draw_step(std::ptrdiff_t w, std::ptrdiff_t h)
{
    init();
    bgr121_image_t img(w, h);
    {
        auto v = view(img);
        fill(v.begin(), v.end(), bgr121_red);
        save_dump(view(img), "dump1");
    }
    {
        auto v = view(img);
        auto loc = v.xy_at(0, v.height() - 1);
        *loc = bgr121_blue; // red red blue
        ++loc.x();
        --loc.y();
        *loc = bgr121_blue; // red blue red
        ++loc.x();
        --loc.y();
        *loc = bgr121_blue; // blue red red
        ++loc.x();
        --loc.y();
        save_dump(view(img), "dump2_step");
    }
}

int main()
{
    try
    {
        draw_loop(3, 3);
        draw_step(3, 3);

        return EXIT_SUCCESS;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;

    }
    return EXIT_FAILURE;
}
