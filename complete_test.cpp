#include <boost/core/lightweight_test.hpp>
#include <boost/crc.hpp>
#include <boost/gil.hpp>
#include <boost/mp11.hpp>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
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
using bgr121_pixel_t = typename bgr121_view_t::value_type;

bgr121_pixel_t bgr121_red(0), bgr121_blue(0);
std::string output_dir_path;

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
void save_dump(View const& view, std::string checksum, std::string name)
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
    std::string const filename = name + "_" + std::to_string(view.width()) + "x" + std::to_string(view.height());
    std::string const out_path = output_dir_path + '/' + filename + file_suffix + ".txt";

    std::vector<std::tuple<int, int, int>> dump;
    dump.reserve(view.width() * view.height());
    gil::for_each_pixel(view, [&dump](bgr121_pixel_t const& p)
    {
        auto b = (int)get_color(p, gil::blue_t{});
        auto g = (int)get_color(p, gil::green_t{});
        auto r = (int)get_color(p, gil::red_t{});
        dump.emplace_back(r, g, b);
    });

    std::cout << "Dump: " << out_path << std::endl;
    std::cout << checksum << std::endl;
    std::ofstream ofs(out_path);
    ofs << checksum << std::endl;
    for (auto& p : dump)
    {
        ofs << std::get<0>(p) << '\t' << std::get<1>(p) << '\t' << std::get<2>(p) << "\n";
        std::cout << std::get<0>(p) << '\t' << std::get<1>(p) << '\t' << std::get<2>(p) << "\n";
    }
}

void fill_image_red(bgr121_image_t& img)
{
    gil::rgb8_pixel_t red8(255, 0, 0);
    gil::rgb8_pixel_t blue8(0, 0, 255);
    gil::color_convert(red8, bgr121_red);
    gil::color_convert(blue8, bgr121_blue);

    auto v = view(img);
    fill(v.begin(), v.end(), bgr121_red);

    for (auto it = view(img).begin().x(), end = view(img).end().x(); it != end; ++it)
        BOOST_TEST(*it == bgr121_red);

    auto const sum = checksum(view(img));
    //save_dump(view(img), sum, "fill_image_red_dump1");
    BOOST_TEST(sum == "23a6f403");
}
// TESTS ///////////////////////////////////////////////////////////////////////
// 1. Fill image with red.
// 2. Draw a blue line along the diagonal.
// 3. Calculate checksum.

void test_draw_with_xy_locator_loop_fail(std::ptrdiff_t w, std::ptrdiff_t h)
{
    bgr121_image_t img(w, h);
    fill_image_red(img);
    {
        auto v = view(img);
        auto loc = v.xy_at(0, v.height() - 1);
        for (std::ptrdiff_t y = 0; y < v.height(); ++y)
        {
            *loc = bgr121_blue;
            // OPTION 1 FAILS
            ++loc.x(); // INCREMENT X FIRST
            --loc.y();

            // OPTION 2 FAILS TOO
            //loc.x()++; // INCREMENT X FIRST
            //loc.y()--;

            // OPTION 3 FAILS TOO
            //auto& x_it = loc.x(); // INCREMENT X FIRST
            //++x_it;
            //auto& y_it = loc.y();
            //--y_it;
        }
    }
    {
        auto const sum = checksum(view(img));
        save_dump(view(img), sum, "xy_locator_loop");
        BOOST_TEST(sum == "2e4950b4");

        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        // row 2
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red);
    }
}

void test_draw_with_xy_locator_loop_good(std::ptrdiff_t w, std::ptrdiff_t h)
{
    bgr121_image_t img(w, h);
    fill_image_red(img);
    {
        auto v = view(img);
        auto loc = v.xy_at(0, v.height() - 1);
        for (std::ptrdiff_t y = 0; y < v.height(); ++y)
        {
            *loc = bgr121_blue;
            // OPTION 1 WORKS
            --loc.y();
            ++loc.x(); // INCREMENT X SECOND

            // OPTION 2  WORKS TOO
            //loc.y()--;
            //loc.x()++; // INCREMENT X SECOND
        }
    }
    {
        auto const sum = checksum(view(img));
        save_dump(view(img), sum, "xy_locator_loop");
        BOOST_TEST(sum == "2e4950b4");

        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        // row 2
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red);
    }
}

void test_draw_with_xy_locator_step_good(std::ptrdiff_t w, std::ptrdiff_t h)
{
    bgr121_image_t img(w, h);
    fill_image_red(img);
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
    }
    {
        auto const sum = checksum(view(img));
        save_dump(view(img), sum, "xy_locator_step");
        BOOST_TEST(sum == "2e4950b4");

        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        // row 2
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red);
    }
}

void test_draw_with_x_iterator_step_good(std::ptrdiff_t w, std::ptrdiff_t h)
{
    bgr121_image_t img(w, h);
    fill_image_red(img);
    {
        auto it = view(img).begin().x();
        // row 0
        *it = bgr121_red; ++it;
        *it = bgr121_red; ++it;
        *it = bgr121_blue; ++it;
        // row 1
        *it = bgr121_red; ++it;
        *it = bgr121_blue; ++it;
        *it = bgr121_red; ++it;
        // row 2
        *it = bgr121_blue; ++it;
        *it = bgr121_red; ++it;
        *it = bgr121_red;
    }
    {
        auto const sum = checksum(view(img));
        save_dump(view(img), sum, "x_iterator_step");
        BOOST_TEST(sum == "2e4950b4");

        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        // row 2
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red);
    }
}

void test_draw_with_xy_operator_step_good(std::ptrdiff_t w, std::ptrdiff_t h)
{
    bgr121_image_t img(w, h);
    fill_image_red(img);
    {
        auto v = view(img);
        // row 0
        v(0, 0) = bgr121_red;
        v(1, 0) = bgr121_red;
        v(2, 0) = bgr121_blue;
        // row 1
        v(0, 1) = bgr121_red;
        v(1, 1) = bgr121_blue;
        v(2, 1) = bgr121_red;
        // row 2
        v(0, 2) = bgr121_blue;
        v(1, 2) = bgr121_red;
        v(2, 2) = bgr121_red;
    }
    {
        auto const sum = checksum(view(img));
        save_dump(view(img), sum, "xy_operator_step");

        BOOST_TEST(sum == "2e4950b4");
        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        // row 2
        BOOST_TEST(*it == bgr121_blue); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red);
    }
}

int main(int argc , char* argv[])
{
    try
    {
        if (argc != 2) throw std::invalid_argument("path to output directory missing");
        output_dir_path = argv[1];

        test_draw_with_xy_locator_loop_fail(3, 3);
        test_draw_with_xy_locator_step_good(3, 3);
        test_draw_with_x_iterator_step_good(3, 3);
        test_draw_with_xy_operator_step_good(3, 3); // immediate pixel reference
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;

    }
    return boost::report_errors();
}
