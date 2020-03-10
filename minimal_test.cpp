// Test:
// 1. Fill image with red.
// 2. Draw a blue line along the diagonal.
// 3. Verify diagonal pixels are blue.
#include <boost/core/lightweight_test.hpp>
#include <boost/gil.hpp>
namespace gil = boost::gil;

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
}

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
            ++loc.x(); // PRE-INCREMENT X FIRST
            --loc.y();

            // OPTION 2 FAILS TOO
            //loc.x()++; // POST-INCREMENT X FIRST
            //loc.y()--;

            // OPTION 3 FAILS TOO
            //auto& x_it = loc.x(); // PRE-INCREMENT NAMED X FIRST
            //++x_it;
            //auto& y_it = loc.y();
            //--y_it;
        }
    }
    {
        auto it = view(img).begin().x();
        // row 0
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it;
        // row 1
        BOOST_TEST(*it == bgr121_red); ++it;
        BOOST_TEST(*it == bgr121_blue); ++it; // FAIL
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
            ++loc.x(); // PRE-INCREMENT X SECOND

            // OPTION 2  WORKS TOO
            //loc.y()--;
            //loc.x()++; // POST-INCREMENT X SECOND
        }
    }
    {
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

int main()
{
    try
    {
        test_draw_with_xy_locator_loop_fail(3, 3);
        test_draw_with_xy_locator_loop_good(3, 3);
        test_draw_with_xy_locator_step_good(3, 3);
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;

    }
    return boost::report_errors();
}
