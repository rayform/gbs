#include <gtest/gtest.h>
#include <gbs/curves>
#include <gbs/maths.h>
#include <gbs/bscbuild.h>
#include <gbs/bscapprox.h>
#include <gbs/vecop.h>
#include <gbs/bscanalysis.h>
#include <gbs-render/vtkcurvesrender.h>
#include <numbers>
namespace
{
    const bool PLOT_ON = true;
}

using gbs::operator-;

TEST(tests_curves, curve2d_rational_offset)
{
    auto circle1 = gbs::build_circle<double, 2>(1.);
    auto f_offset = gbs::BSCfunction<double>(gbs::build_segment<double, 1>({1.}, {1.}));
    auto f_offset3 = gbs::BSCfunction<double>(gbs::build_segment<double, 1>({1.}, {2.}));
    auto f_offset4 = gbs::BSCfunction<double>(gbs::BSCurve<double, 1>(
        gbs::points_vector<double,1>{{0.},{0.},{1.},{0.},{0.}},
        {0.,0.,0.,0.1,0.5,1.,1.,1.},
        2
        )
    );
    auto p_circle1 = std::make_shared<gbs::BSCurveRational<double, 2>>(circle1);
    gbs::CurveOffset<double, 2,gbs::BSCfunction<double>> circle2{
        p_circle1,
        f_offset};
    gbs::CurveOffset<double, 2,gbs::BSCfunction<double>> circle3{
        p_circle1,
        f_offset3};
    gbs::CurveOffset<double, 2,gbs::BSCfunction<double>> circle4{
        p_circle1,
        f_offset4};
    auto u = gbs::deviation_based_params<double, 2>(circle1, 30, 0.01);
    for (auto u_ : u)
    {
        ASSERT_NEAR(gbs::norm(circle1(u_) - circle2(u_)), 1., 1e-6);
        ASSERT_NEAR(gbs::norm(circle1(u_) - circle3(u_)), f_offset3(u_), 1e-6);
        ASSERT_NEAR(gbs::norm(circle1(u_) - circle4(u_)), f_offset4(u_), 1e-6);
    }

    if (PLOT_ON)
        gbs::plot(
            gbs::crv_dsp<double, 2, true>{
                .c = &(circle1),
                .col_crv = {0., 0., 0.},
                .poles_on = true,
                .line_width = 3.,
            },
            gbs::make_actor(circle2, {1., 0., 0.}),
            gbs::make_actor(circle3, {0., 1., 0.}),
            gbs::make_actor(circle4, {0., 0., 1.}) );
}

TEST(tests_curves, curve2d_offset)
{
    std::string line;
    std::ifstream myfile("../tests/in/e1098.dat");
    // std::ifstream myfile("../tests/in/e817.dat");
    if (myfile.is_open())
    {
        std::vector<std::array<double, 2>> pts;
        getline(myfile, line);
        while (getline(myfile, line))
        {
            std::istringstream iss(line);
            std::string::size_type sz; // alias of size_t

            double x = std::stod(line, &sz);
            double y = std::stod(line.substr(sz));
            pts.push_back({x, y});
        }
        myfile.close();

        const auto d_offset {0.2};
        auto crv = gbs::approx(pts, 5, gbs::KnotsCalcMode::CHORD_LENGTH, true);
        auto f_offset = gbs::BSCfunction<double>(gbs::build_segment<double, 1>({d_offset}, {d_offset}));
        auto p_crv = std::make_shared<gbs::BSCurveRational<double, 2>>(crv);
        gbs::CurveOffset<double, 2,gbs::BSCfunction<double>> crv_offset{
            p_crv,
            f_offset};

        auto u = gbs::deviation_based_params<double,2>(crv,30,0.01);
        for(auto u_ :u)
        {
            ASSERT_NEAR(gbs::norm(crv(u_)-crv_offset(u_)),d_offset,1e-6);
        }

        if (PLOT_ON)
            gbs::plot(
        gbs::crv_dsp<double, 2, false>{
            .c = &(crv),
            .col_crv = {0,0,1},
            // .poles_on = false,
            .poles_on = true,
            .line_width=3.,
            .show_curvature=false,
         }
                ,
                crv_offset);
    }
}
TEST(tests_curves, curve2d_offset_functor)
{
    auto circle1 = gbs::build_circle<double, 2>(1.);
    size_t n_pulse = 10;
    auto f_offset = [n_pulse](auto u, size_t d = 0){return d==0 ? std::sin(u*2.*std::numbers::pi*n_pulse)*0.5 + 0.5 : std::numbers::pi*n_pulse*std::cos(u*2.*std::numbers::pi*n_pulse);};
    auto p_circle1 = std::make_shared<gbs::BSCurveRational<double, 2>>(circle1);
    gbs::CurveOffset<double, 2,decltype(f_offset)> circle2{
        p_circle1,
        f_offset};

    auto u = gbs::deviation_based_params<double, 2>(circle1, 100, 0.01);
    for (auto u_ : u)
    {
        ASSERT_NEAR(gbs::norm(circle1(u_) - circle2(u_)), f_offset(u_), 1e-6);
    }

    if (PLOT_ON)
        gbs::plot(
            gbs::crv_dsp<double, 2, true>{
                .c = &(circle1),
                .col_crv = {0., 0., 0.},
                .poles_on = true,
                .line_width = 3.,
            },
            gbs::make_actor(circle2, {1., 0., 0.}) );
}

TEST(tests_curves,curve_on_surface)
{
    const std::vector<std::array<double,3> > points =
    {
        {0,0,0},{1,0,0},
        {0,1,0},{1,1,1},
        {0,2,1},{2,1,0},
        {3,2,0},{3,2,0},
    };
    std::vector<double> ku = {0.,0.,1.,1.};
    std::vector<double> kv = {0.,0.,0.,0.5,1.,1.,1.};
    size_t p = 1;
    size_t q = 2;
    std::vector<double> u = {0.,1.};
    std::vector<double> v = {0.,0.33,0.66,1.};
    auto poles = gbs::build_poles(points,ku,kv,u,v,p,q);

    gbs::BSSurface srf(poles,ku,kv,p,q) ;

    auto r_values = gbs::make_range<double>(0.05,0.45,10);
    std::vector<gbs::CurveOnSurface<double,3>> crv_lst;
    auto p_srf = std::make_shared<gbs::BSSurface<double,3>>(srf);
    for(auto r : r_values)
    {
        auto cir2d = gbs::build_circle<double,2>(r,{0.5,0.5});
        crv_lst.push_back({ std::make_shared<gbs::BSCurveRational<double,2>>(cir2d), p_srf});
    }

    // gbs::CurveOnSurface<double,3> crv_on_srf { std::make_shared<gbs::BSCurveRational<double,2>>(cir2d), std::make_shared<gbs::BSSurface<double,3>>(srf)};

    // crv_on_srf.value(0.5);

    size_t i = crv_lst.size() / 2 ;
    size_t n_pulse = 10;
    auto f_offset = [n_pulse,amp=0.03](auto u, size_t d = 0)
    {
        return d==0 ? std::sin(u*2.*std::numbers::pi*n_pulse)*amp + amp : std::numbers::pi*n_pulse*std::cos(u*2.*std::numbers::pi*n_pulse);
    };
    gbs::CurveOffset<double, 2, decltype(f_offset)> circle2{
        std::make_shared<gbs::BSCurveRational<double, 2>>(
            gbs::build_circle<double,2>(r_values[i],{0.5,0.5})
        ),
        f_offset
    };

    gbs::CurveOnSurface<double,3> crv_o {
        std::make_shared<gbs::CurveOffset<double,2, decltype(f_offset)>>(circle2),
        p_srf
    };

    size_t deg = 5;
    size_t n_poles = 200;
    // auto crv_o_approx1 = gbs::approx(crv_o,0.05,deg,gbs::KnotsCalcMode::CHORD_LENGTH);
    auto crv_o_approx2 = gbs::approx(crv_o,0.05,n_poles,deg,gbs::KnotsCalcMode::CHORD_LENGTH);

    auto actor = gbs::make_actor(
                crv_o_approx2,
                {0.0,0.,0.0});

    gbs::StippledLine(actor);

    if (PLOT_ON)
        gbs::plot(
            srf 
            ,
            crv_lst
            // ,
            // gbs::make_curvature(crv_lst[i],0.2,100)
            ,
            gbs::make_actor(
                crv_lst[i],
                {0.,0.,1.}
            )
            ,
            // gbs::make_actor(
            //     crv_o_approx1,
            //     {0.3,1.,0.7}
            // )
            // ,
            gbs::make_actor(
                crv_o,
                {0.,1.,0.}
            )
            ,
            actor
            );

}