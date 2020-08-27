#pragma once
#include <gbslib/bscurve.h>
#include <gbslib/transform.h>

namespace gbs
{
    template <typename T, size_t dim>
    auto build_ellipse(T radius1,T radius2,const std::array<T, dim> &center = std::array<T, dim>{} ) -> gbs::BSCurve<T, dim + 1>
    {
        std::vector<T> k = {0., 0., 0., 1. / 4., 1. / 4., 1. / 2., 1. / 2., 3. / 4., 3. / 4., 1., 1., 1.};
        auto wi = sqrt(2.) / 2.;

        std::vector<std::array<T, dim + 1>> poles(9);
        poles[0][0] = radius1;
        poles[0].back() = 1.;

        poles[1][0] = wi * radius1;
        poles[1][1] = wi * radius2;
        poles[1].back() = wi;

        poles[2][1] = radius2;
        poles[2].back() = 1;

        poles[3][0] = -wi * radius1;
        poles[3][1] = wi * radius2;
        poles[3].back() = wi;

        poles[4][0] = -radius1;
        poles[4].back() = 1;

        poles[5][0] = -wi * radius1;
        poles[5][1] = -wi * radius2;
        poles[5].back() = wi;

        poles[6][1] = -radius2;
        poles[6].back() = 1;

        poles[7][0] = wi * radius1;
        poles[7][1] = -wi * radius2;
        poles[7].back() = wi;

        poles[8][0] = radius1;
        poles[8].back() = 1;

        std::transform(
            poles.begin(), poles.end(),
            poles.begin(),
            [&center](auto p_) {
                translate(p_, center);
                return p_;
            });

        size_t p = 2;

        return gbs::BSCurve(poles, k, p);
    }
    template <typename T, size_t dim>
    auto build_circle(T radius,const std::array<T, dim> &center = std::array<T, dim>{} ) -> gbs::BSCurve<T, dim + 1>
    {
        return build_ellipse(radius,radius,center);
     }

} // namespace gbs