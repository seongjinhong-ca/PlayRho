/*
 * Copyright (c) 2023 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "UnitTests.hpp"

#include <PlayRho/Common/Math.hpp>

#include <PlayRho/Dynamics/Contacts/ConstraintSolverConf.hpp>

#include <array>
#include <type_traits>
#include <chrono>
#include <cmath>

using namespace playrho;
using namespace playrho::d2;

namespace {

Angle AlternateGetShortestDelta(Angle a0, Angle a1) noexcept
{
    constexpr auto twoPi = Angle{Pi * 2 * Radian};
    const auto da = ModuloViaTrunc(a1 - a0, twoPi);
    return ModuloViaTrunc(2 * da, twoPi) - da;
}

} // namespace

TEST(Math, std_sqrt)
{
    EXPECT_EQ(sqrt(Real{0}), Real{0});
    EXPECT_EQ(sqrt(Real{4}), Real{2});
    EXPECT_EQ(sqrt(Real{25}), Real{5});
    EXPECT_NE(sqrt(std::numeric_limits<Real>::min()), Real(0));

    EXPECT_NE(std::sqrt(std::numeric_limits<double>::min()), double(0));
    EXPECT_EQ(Square(std::sqrt(std::numeric_limits<double>::min())), std::numeric_limits<double>::min());

    // The sin/cos of a 45 degree angle...
    EXPECT_EQ(sqrt(1.0 / 2.0), 0.70710678118654752440);
    EXPECT_EQ(sqrt(1.0l / 2.0l), 0.70710678118654752440l);
    EXPECT_EQ(sqrt(2.0L) / 2.0L, 0.70710678118654752440L);
    EXPECT_EQ(sqrt(2.0L), 1.414213562373095048801688724209698078569671875376948073176679737990732478462L);
}

TEST(Math, std_atan2)
{
    // atan2 range appears to be (-PI, +PI]
    const auto PI = 3.14159265358979323846264338327950288;
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, -1.0), +1.0), +PI * 0.0);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, +0.0), +1.0), +PI * 0.0);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, +1.0), +1.0), +PI * 0.0);
    EXPECT_DOUBLE_EQ(atan2(+1.0, +0.0), +PI / 2.0);
    EXPECT_DOUBLE_EQ(atan2(-1.0, +0.0), -PI / 2.0);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, -1.0), -1.0), -PI);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, -0.0), -1.0), -PI);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, +0.0), -1.0), +PI);
    EXPECT_DOUBLE_EQ(atan2(nextafter(0.0, +1.0), -1.0), +PI);
}

TEST(Math, Square)
{
    ASSERT_NE(std::numeric_limits<float>::min() * 2, std::numeric_limits<float>::min());

    EXPECT_EQ(Square(std::numeric_limits<float>::min()), float(0));
    EXPECT_EQ(Square(std::numeric_limits<float>::min() * float(2251799947902976)), float(0));
    EXPECT_NE(Square(std::numeric_limits<float>::min() * float(2251799947902977)), float(0));

    auto low = float(0);
    auto high = float(0);
    auto value = float(0);

    low = std::numeric_limits<float>::min() * float(2251799947902976);
    high = std::numeric_limits<float>::min() * float(2251799947902977);
    do
    {
        value = (low + high) / float(2);
        if ((value == low) || (value == high))
        {
            break;
        }
        if (Square(value) != float(0))
        {
            high = value;
        }
        else
        {
            low = value;
        }
    }
    while (low < high);
    
#if 0
    std::cout << "Min float is: ";
    std::cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
    std::cout << std::numeric_limits<float>::min();
    std::cout << " aka ";
    std::cout << std::hexfloat;
    std::cout << std::numeric_limits<float>::min();
    std::cout << std::defaultfloat;
    std::cout << std::endl;

    std::cout << "Least float that squared isn't zero: ";
    std::cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
    std::cout << high;
    std::cout << " aka ";
    std::cout << std::hexfloat;
    std::cout << high;
    std::cout << std::defaultfloat;
    std::cout << std::endl;
#endif
    EXPECT_EQ(high, float(2.646978275714050648e-23));

    ASSERT_NE(Square(high), float(0));
    ASSERT_EQ(sqrt(Square(float(1))), float(1));
#if 0
    std::cout << "sqrt(min) is: ";
    std::cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
    std::cout << sqrt(std::numeric_limits<float>::min());
    std::cout << " aka ";
    std::cout << std::hexfloat;
    std::cout << sqrt(std::numeric_limits<float>::min());
    std::cout << std::endl;
    EXPECT_EQ(sqrt(std::numeric_limits<float>::min()), float(0x1p-63)); // float(1.084202172485504434e-19)
#endif
    
    // What is the smallest float a for which:
    // AlmostEqual(sqrt(square(a)), a) and AlmostEqual(square(sqrt(a)), a)
    // hold true?
    
    const auto a = sqrt(std::numeric_limits<float>::min());
    EXPECT_TRUE(AlmostEqual(Square(sqrt(a)), a));
    EXPECT_TRUE(AlmostEqual(sqrt(Square(a)), a));
}

TEST(Math, Atan2)
{
    EXPECT_EQ(Atan2(Real(0), Real(0)), 0_deg);
    //EXPECT_EQ(Atan2(Real(1), Real(0)), 90_deg);
}

TEST(Math, Span)
{
    {
        const auto vector = Vector<int, 3>{1, 2, 4};
        const Span<const int> foo = Span<const int>(vector);
        EXPECT_EQ(foo.size(), std::size_t(3));
        EXPECT_EQ(foo[0], 1);
        EXPECT_EQ(foo[1], 2);
        EXPECT_EQ(foo[2], 4);
    }
    {
        // check initialization from explicit initializer list
        const auto initList = std::initializer_list<int>{1, 2, 4};
        const auto foo = Span<const int>(initList);
        EXPECT_EQ(foo.size(), std::size_t(3));
        EXPECT_EQ(foo[0], 1);
        EXPECT_EQ(foo[1], 2);
        EXPECT_EQ(foo[2], 4);
    }
    {
        // check initialization from non-const array
        int array[6] = {1, 2, 4, 10, -1, -33};
        auto foo = Span<int>(array);
        EXPECT_EQ(foo.size(), std::size_t(6));
        EXPECT_EQ(foo[0], 1);
        EXPECT_EQ(foo[1], 2);
        EXPECT_EQ(foo[2], 4);
        EXPECT_EQ(foo[3], 10);
        EXPECT_EQ(foo[4], -1);
        EXPECT_EQ(foo[5], -33);
        foo[3] = 22;
        EXPECT_EQ(foo[3], 22);
    }
    {
        float array[15];
        EXPECT_EQ(Span<float>(array).size(), std::size_t(15));
        EXPECT_EQ(Span<float>(array, 2).size(), std::size_t(2));        
    }
}

TEST(Math, Average)
{
    EXPECT_EQ(Average(std::initializer_list<int>{}), 0);
    EXPECT_EQ(Average(std::initializer_list<float>{}), float(0));

    EXPECT_EQ(Average(std::initializer_list<int>{0}), 0);
    EXPECT_EQ(Average(std::initializer_list<int>{4}), 4);
    EXPECT_EQ(Average(std::initializer_list<int>{-3}), -3);
    EXPECT_EQ(Average(std::initializer_list<float>{float(-3)}), float(-3));

    EXPECT_EQ(Average(std::initializer_list<int>{0, 0}), 0);
    EXPECT_EQ(Average(std::initializer_list<int>{2, 2}), 2);
    EXPECT_EQ(Average(std::initializer_list<int>{2, 4}), 3);
    EXPECT_EQ(Average(std::initializer_list<float>{float(2), float(3)}), float(2.5));

    EXPECT_EQ(Average(std::initializer_list<int>{2, 4, 6}), 4);
    EXPECT_EQ(Average(std::initializer_list<int>{2, 4, 12}), 6);
    EXPECT_EQ(Average(std::initializer_list<double>{2.0, 4.0, 6.0}), 4.0);
    EXPECT_EQ(Average(std::initializer_list<double>{2.0, 4.0, 12.0}), 6.0);
    
    EXPECT_EQ(Average(std::array<double, 3>{{2.0, 4.0, 12.0}}), 6.0);
    EXPECT_EQ(Average(std::vector<double>{2.0, 4.0, 12.0}), 6.0);
}

TEST(Math, AverageVec2)
{
    EXPECT_EQ(Average(std::initializer_list<Vec2>{}), Vec2(0, 0));
    
    {
        const auto val = Vec2{Real(3.9), Real(-0.1)};
        EXPECT_EQ(Average(std::initializer_list<Vec2>{val}), val);
    }
    
    {
        const auto val1 = Vec2{Real(2.2), Real(-1.1)};
        const auto val2 = Vec2{Real(4.4), Real(-1.3)};
        const auto average = Average(std::initializer_list<Vec2>{val1, val2});
        const auto expected = Vec2(Real(3.3), Real(-1.2));
        EXPECT_NEAR(double(GetX(average)), double(GetX(expected)), 0.0001);
        EXPECT_NEAR(double(GetY(average)), double(GetY(expected)), 0.0001);
    }
}

TEST(Math, AverageLength2)
{
    EXPECT_EQ(Average(std::initializer_list<Length2>{}), Length2(0_m, 0_m));
    EXPECT_EQ(Average(std::initializer_list<Length2>{{1_m, 2_m}}), Length2(1_m, 2_m));
    EXPECT_EQ(Average(std::initializer_list<Length2>{{1_m, 2_m},{-1_m, 2_m}}), Length2(0_m, 2_m));
    EXPECT_EQ(Average(std::initializer_list<Length2>{{1_m, 2_m},{1_m, -2_m}}), Length2(1_m, 0_m));
    EXPECT_EQ(Average(std::initializer_list<Length2>{{1_m, 2_m},{-1_m, -2_m}}), Length2(0_m, 0_m));
    EXPECT_EQ(Average(std::initializer_list<Length2>{
                {3_m, 2_m},{-3_m, 2_m},{-3_m, -2_m},{3_m, -2_m}
              }), Length2(0_m, 0_m));
}

TEST(Math, DotProductOfTwoVecTwoIsCommutative)
{
    const auto a = Vec2{Real(-3.2), Real(1.9)};
    const auto b = Vec2{Real(4.01), Real(-0.002)};
    EXPECT_EQ(Dot(a, b), Dot(b, a));
}

TEST(Math, DotProductOfTwoVecThreeIsCommutative)
{
    const auto a = Vec3{Real(-3.2), Real(1.9), Real(36.01)};
    const auto b = Vec3{Real(4.01), Real(-0.002), Real(1.2)};
    EXPECT_EQ(Dot(a, b), Dot(b, a));
}

TEST(Math, CrossProductOfTwoVecTwoIsAntiCommutative)
{
    const auto a = Vec2{Real(-3.2), Real(1.9)};
    const auto b = Vec2{Real(4.01), Real(-0.002)};
    EXPECT_EQ(Cross(a, b), -Cross(b, a));
}

TEST(Math, DotProductOfInvalidIsInvalid)
{
    EXPECT_TRUE(isnan(Dot(GetInvalid<Vec2>(), GetInvalid<Vec2>())));

    EXPECT_TRUE(isnan(Dot(Vec2(0, 0), GetInvalid<Vec2>())));
    EXPECT_TRUE(isnan(Dot(Vec2(0, 0), Vec2(GetInvalid<Real>(), 0))));
    EXPECT_TRUE(isnan(Dot(Vec2(0, 0), Vec2(0, GetInvalid<Real>()))));
    
    EXPECT_TRUE(isnan(Dot(GetInvalid<Vec2>(),             Vec2(0, 0))));
    EXPECT_TRUE(isnan(Dot(Vec2(GetInvalid<Real>(), 0), Vec2(0, 0))));
    EXPECT_TRUE(isnan(Dot(Vec2(0, GetInvalid<Real>()), Vec2(0, 0))));

    EXPECT_TRUE(isnan(Dot(GetInvalid<Vec2>(), GetInvalid<UnitVec>())));
    //EXPECT_TRUE(isnan(Dot(Vec2(0, 0),         GetInvalid<UnitVec>())));
    EXPECT_TRUE(isnan(Dot(GetInvalid<Vec2>(), UnitVec::GetZero())));

    EXPECT_TRUE(isnan(Dot(GetInvalid<UnitVec>(), GetInvalid<Vec2>())));
    //EXPECT_TRUE(isnan(Dot(GetInvalid<UnitVec>(), Vec2(0, 0))));
    EXPECT_TRUE(isnan(Dot(UnitVec::GetZero(),    GetInvalid<Vec2>())));
}

TEST(Math, Vec2NegationAndRotationIsOrderIndependent)
{
    {
        const auto v = Vec2{Real(1), Real(1)};
        const auto r = UnitVec::GetRight();
        EXPECT_EQ(Rotate(-v, r), -Rotate(v, r));
    }
    {
        const auto v = Vec2{Real(1), Real(1)};
        const auto r = UnitVec::Get(33_deg);
        EXPECT_EQ(Rotate(-v, r), -Rotate(v, r));
    }
    {
        const auto v = Vec2{Real(-3.2), Real(1.9)};
        const auto r = UnitVec::Get(33_deg);
        EXPECT_EQ(Rotate(-v, r), -Rotate(v, r));
    }
    {
        const auto v = Vec2{Real(-3.2), Real(-21.4)};
        for (auto angle = -360_deg; angle < 360_deg; angle += 15_deg)
        {
            const auto r = UnitVec::Get(angle);
            EXPECT_EQ(Rotate(-v, r), -Rotate(v, r));
        }
    }
    {
        const auto v = Vec2{Real(-3.2), Real(1.9)};
        const auto r = UnitVec::Get(33_deg);
        EXPECT_EQ(Rotate(v, r), -Rotate(-v, r));
    }
    {
        const auto v = Vec2{Real(-3.2), Real(1.9)};
        const auto r = UnitVec::Get(33_deg);
        EXPECT_EQ(Rotate(v, r), -Rotate(v, -r));
    }
}

TEST(Math, InverseRotationRevertsRotation)
{
    const auto vec_list = {Vec2{-10.7f, 5.3f}, Vec2{3.2f, 21.04f}, Vec2{-1.2f, -0.78f}};
    for (auto&& vec: vec_list) {
        for (auto angle = 0_deg; angle < 360_deg; angle += 10_deg)
        {
            const auto unit_vec = UnitVec::Get(angle);
            EXPECT_NEAR(double(GetX(InverseRotate(Rotate(vec, unit_vec), unit_vec))), double(GetX(vec)), 0.004);
            EXPECT_NEAR(double(GetY(InverseRotate(Rotate(vec, unit_vec), unit_vec))), double(GetY(vec)), 0.004);
        }
    }
}

TEST(Math, TransformIsRotatePlusTranslate)
{
    const auto vector = Length2{19_m, -0.5_m};
    const auto translation = Length2{-3_m, +5_m};
    const auto rotation = UnitVec::GetTop();
    const auto transformation = Transformation{translation, rotation};
    
    const auto transformed_vector = Transform(vector, transformation);
    const auto alt = Rotate(vector, rotation) + translation;
    
    EXPECT_EQ(transformed_vector, alt);
}

TEST(Math, InverseTransformIsUntranslateAndInverseRotate)
{
    const auto vector = Length2{19_m, -0.5_m};
    const auto translation = Length2{-3_m, +5_m};
    const auto rotation = UnitVec::GetTop();
    const auto transformation = Transformation{translation, rotation};
    
    const auto inv_vector = InverseTransform(vector, transformation);
    const auto alt = InverseRotate(vector - translation, rotation);
    
    EXPECT_EQ(inv_vector, alt);
}

TEST(Math, InverseTransformTransformedIsOriginal)
{
    const auto vector = Length2{19_m, -0.5_m};
    const auto translation = Length2{-3_m, +5_m};
    const auto rotation = UnitVec::GetTop();
    const auto transformation = Transformation{translation, rotation};

    const auto transformed_vector = Transform(vector, transformation);
    const auto inverse_transformed_vector = InverseTransform(transformed_vector, transformation);

    EXPECT_NEAR(double(Real{GetX(vector) / Meter}),
                double(Real{GetX(inverse_transformed_vector) / Meter}), 0.0001);
    EXPECT_NEAR(double(Real{GetY(vector) / Meter}),
                double(Real{GetY(inverse_transformed_vector) / Meter}), 0.0001);
}

TEST(Math, TransformInverseTransformedIsOriginal)
{
    const auto vector = Length2{19_m, -0.5_m};
    const auto translation = Length2{-3_m, +5_m};
    const auto rotation = UnitVec::GetTop();
    const auto transformation = Transformation{translation, rotation};

    const auto inverse_transformed_vector = InverseTransform(vector, transformation);
    const auto transformed_inverse_vector = Transform(inverse_transformed_vector, transformation);
    
    EXPECT_NEAR(double(Real{GetX(vector) / Meter}),
                double(Real{GetX(transformed_inverse_vector) / Meter}), 0.00001);
    EXPECT_NEAR(double(Real{GetY(vector) / Meter}),
                double(Real{GetY(transformed_inverse_vector) / Meter}), 0.00001);
}

TEST(Math, ComputeCentroidCenteredR1)
{
    const auto hx = Real(1);
    const auto hy = Real(1);
    const auto real_center = Vec2{0, 0};
    const auto vertices = {
        (real_center + Vec2{hx, hy}) * Meter,
        (real_center + Vec2{-hx, +hy}) * Meter,
        (real_center - Vec2{hx, hy}) * Meter,
        (real_center + Vec2{+hx, -hy}) * Meter,
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_EQ(GetX(center), GetX(real_center) * Meter);
    EXPECT_EQ(GetY(center), GetY(real_center) * Meter);
    
    const auto average = Average(vertices);
    EXPECT_EQ(average, center);
}

TEST(Math, ComputeCentroidCentered0R1000)
{
    const auto hx = Real(1000);
    const auto hy = Real(1000);
    const auto real_center = Vec2{0, 0};
    const auto vertices = {
        (real_center + Vec2{hx, hy}) * Meter,
        (real_center + Vec2{-hx, +hy}) * Meter,
        (real_center + Vec2{-hx, -hy}) * Meter,
        (real_center + Vec2{+hx, -hy}) * Meter
    };
    const auto center = ComputeCentroid(vertices);
    
    EXPECT_EQ(GetX(center), GetX(real_center) * Meter);
    EXPECT_EQ(GetY(center), GetY(real_center) * Meter);
    
    const auto average = Average(vertices);
    EXPECT_EQ(average, center);
}

TEST(Math, ComputeCentroidUpRight1000R1)
{
    const auto hx = Real(1);
    const auto hy = Real(1);
    const auto real_center = Vec2{1000, 1000};
    const auto vertices = {
        (real_center + Vec2{+hx, +hy}) * Meter,
        (real_center + Vec2{-hx, +hy}) * Meter,
        (real_center + Vec2{-hx, -hy}) * Meter,
        (real_center + Vec2{+hx, -hy}) * Meter
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_NEAR(double(Real{GetX(center) / Meter}), double(GetX(real_center)), 0.01);
    EXPECT_NEAR(double(Real{GetY(center) / Meter}), double(GetY(real_center)), 0.01);
    
    const auto average = Average(vertices);
    EXPECT_NEAR(double(Real{GetX(average) / Meter}), double(Real{GetX(center) / Meter}), 0.01);
    EXPECT_NEAR(double(Real{GetY(average) / Meter}), double(Real{GetY(center) / Meter}), 0.01);
}

TEST(Math, ComputeCentroidUpRight1000R100)
{
    const auto hx = Real(100);
    const auto hy = Real(100);
    const auto real_center = Vec2{1000, 1000};
    const auto vertices = {
        (real_center + Vec2{+hx, +hy}) * Meter,
        (real_center + Vec2{-hx, +hy}) * Meter,
        (real_center + Vec2{-hx, -hy}) * Meter,
        (real_center + Vec2{+hx, -hy}) * Meter
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_NEAR(double(Real{GetX(center) / Meter}), double(GetX(real_center)), 0.01);
    EXPECT_NEAR(double(Real{GetY(center) / Meter}), double(GetY(real_center)), 0.01);
    
    const auto average = Average(vertices);
    EXPECT_NEAR(double(Real{GetX(average) / Meter}), double(Real{GetX(center) / Meter}), 0.01);
    EXPECT_NEAR(double(Real{GetY(average) / Meter}), double(Real{GetY(center) / Meter}), 0.01);
}

TEST(Math, ComputeCentroidUpRight10000R01)
{
    const auto hx = Real(0.1);
    const auto hy = Real(0.1);
    const auto real_center = Vec2{10000, 10000};
    const auto vertices = {
        (real_center + Vec2{+hx, +hy}) * Meter,
        (real_center + Vec2{-hx, +hy}) * Meter,
        (real_center + Vec2{-hx, -hy}) * Meter,
        (real_center + Vec2{+hx, -hy}) * Meter
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_NEAR(double(Real{GetX(center) / Meter}), double(GetX(real_center)), 0.1);
    EXPECT_NEAR(double(Real{GetY(center) / Meter}), double(GetY(real_center)), 0.1);
    
    const auto average = Average(vertices);
    EXPECT_NEAR(double(Real{GetX(average) / Meter}), double(Real{GetX(center) / Meter}), 0.1);
    EXPECT_NEAR(double(Real{GetY(average) / Meter}), double(Real{GetY(center) / Meter}), 0.1);
}

TEST(Math, ComputeCentroidDownLeft1000R1)
{
    const auto hx = Real(1);
    const auto hy = Real(1);
    const auto real_center = Vec2{-1000, -1000};
    const auto vertices = {
        Vec2{GetX(real_center) + hx, GetY(real_center) + hy} * Meter,
        Vec2{GetX(real_center) - hx, GetY(real_center) + hy} * Meter,
        Vec2{GetX(real_center) - hx, GetY(real_center) - hy} * Meter,
        Vec2{GetX(real_center) + hx, GetY(real_center) - hy} * Meter
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_NEAR(double(Real{GetX(center) / Meter}), double(GetX(real_center)), 0.01);
    EXPECT_NEAR(double(Real{GetY(center) / Meter}), double(GetY(real_center)), 0.01);
    
    const auto average = Average(vertices);
    EXPECT_NEAR(double(Real{GetX(average) / Meter}), double(Real{GetX(center) / Meter}), 0.01);
    EXPECT_NEAR(double(Real{GetY(average) / Meter}), double(Real{GetY(center) / Meter}), 0.01);
}

TEST(Math, ComputeCentroidOfHexagonalVertices)
{
    const auto hx = Real(1);
    const auto hy = Real(1);
    const auto real_center = Vec2{-1000, -1000};
    const auto vertices = {
        Vec2{GetX(real_center) + 00, GetY(real_center) + 2 * hy} * Meter,
        Vec2{GetX(real_center) - hx, GetY(real_center) + 1 * hy} * Meter,
        Vec2{GetX(real_center) - hx, GetY(real_center) - 1 * hy} * Meter,
        Vec2{GetX(real_center) + 00, GetY(real_center) - 2 * hy} * Meter,
        Vec2{GetX(real_center) + hx, GetY(real_center) - 1 * hy} * Meter,
        Vec2{GetX(real_center) + hx, GetY(real_center) + 1 * hy} * Meter,
    };
    const auto center = ComputeCentroid(vertices);
    EXPECT_NEAR(double(Real{GetX(center) / Meter}), double(GetX(real_center)), 0.01);
    EXPECT_NEAR(double(Real{GetY(center) / Meter}), double(GetY(real_center)), 0.01);
    
    const auto average = Average(vertices);
    EXPECT_NEAR(double(Real{GetX(average) / Meter}), double(Real{GetX(center) / Meter}), 0.01);
    EXPECT_NEAR(double(Real{GetY(average) / Meter}), double(Real{GetY(center) / Meter}), 0.01);
}

TEST(Math, GetContactRelVelocity)
{
    const auto velA = Velocity{LinearVelocity2(+1_mps, +4_mps), 3.2f * RadianPerSecond};
    const auto velB = Velocity{LinearVelocity2(+3_mps, +1_mps), 0.4f * RadianPerSecond};
    const auto relA = Length2{};
    const auto relB = Length2{};
    const auto result = GetContactRelVelocity(velA, relA, velB, relB);
    
    EXPECT_EQ(result, velB.linear - velA.linear);
}

TEST(Math, NextPowerOfTwo)
{
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{0u}), 1u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{1u}), 2u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{2u}), 4u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{3u}), 4u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{4u}), 8u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{5u}), 8u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{6u}), 8u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{7u}), 8u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{8u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{9u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{10u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{11u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{12u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{13u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{14u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{15u}), 16u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{16u}), 32u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{31u}), 32u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{32u}), 64u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{63u}), 64u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{64u}), 128u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{127u}), 128u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{128u}), 256u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{255u}), 256u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{256u}), 512u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{511u}), 512u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{512u}), 1024u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{1023u}), 1024u);
    EXPECT_EQ(NextPowerOfTwo(std::uint32_t{1024u}), 2048u);
    for (auto i = std::uint32_t{0}; i < 32u; ++i)
    {
        const auto val = (std::uint32_t{1u} << i);
        EXPECT_EQ(NextPowerOfTwo(val - 1u), val);
    }

    constexpr auto max = std::numeric_limits<std::uint32_t>::max() / 512;
    for (auto i = decltype(max){0}; i < max; ++i)
    {
        const auto next = std::pow(2, std::ceil(std::log(i + 1)/std::log(2)));
        EXPECT_EQ(NextPowerOfTwo(i), next);
    }
    EXPECT_EQ(NextPowerOfTwo(static_cast<std::uint64_t>(-1)), 0u);
}

TEST(Math, Subtracting2UlpAlmostEqualNumbersNotAlmostZero)
{
    const auto a = 0.863826155f;
    const auto b = 0.863826453f;
    ASSERT_NE(a, b);
    ASSERT_TRUE(AlmostEqual(a, b, 2));
    ASSERT_FALSE(AlmostEqual(a, b, 1));
    EXPECT_FALSE(AlmostZero((a >= b)? a - b: b - a));
}

TEST(Math, Subtracting1UlpAlmostEqualNumbersIsNotAlmostZero)
{
    const auto a = 0.8638264550000f;
    const auto b = 0.8638264238828f;
    ASSERT_NE(a, b);
    ASSERT_TRUE(AlmostEqual(a, b, 1));
    ASSERT_FALSE(AlmostEqual(a, b, 0));
    EXPECT_FALSE(AlmostZero((a >= b)? a - b: b - a));
}

TEST(Math, nextafter)
{
    const auto a = float(0.863826394);
    const auto b = float(0.863826453);
    
    ASSERT_NE(a, b);
    ASSERT_TRUE(AlmostEqual(a, b, 2));

    const auto ap = std::nextafter(a, a + 1);
    
    EXPECT_NE(a, ap);
    EXPECT_EQ(ap, b);
    EXPECT_EQ((a + b) / 2, a);
}

TEST(Math, nextafter2)
{
    const auto a = 0.863826155f;
    const auto b = std::nextafter(a, 1.0f);
    ASSERT_TRUE(AlmostEqual(a, b, 2));
    ASSERT_TRUE(AlmostEqual(a, b, 1));
    ASSERT_FALSE(AlmostEqual(a, b, 0));
    ASSERT_TRUE(a != b);
    const auto d = b - a;
    ASSERT_FALSE(AlmostZero(d));
    EXPECT_EQ(a + d, b);
    EXPECT_EQ(b - d, a);
    const auto minfloat = std::numeric_limits<float>::min();
    ASSERT_NE(minfloat, 0.0f);
    ASSERT_TRUE(minfloat > 0.0f);
    ASSERT_NE(minfloat, d);
    ASSERT_FALSE(AlmostZero(minfloat));
    const auto subnormal = minfloat / 2;
    ASSERT_TRUE(AlmostZero(subnormal));
    ASSERT_NE(minfloat, subnormal);
    EXPECT_EQ(a + subnormal, a);
    EXPECT_EQ(b + subnormal, b);
}

template <class T>
static void TestModuloFunction(std::function<T(T, T)> f)
{
    constexpr auto abs_error = 1e-6;
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+1.0), static_cast<T>(+1.0))), 0.0, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+1.0), static_cast<T>(+2.0))), 1.0, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+3.0), static_cast<T>(+2.0))), 1.0, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+5.1), static_cast<T>(+3.0))), 2.1, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(-5.1), static_cast<T>(+3.0))), -2.1, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+5.1), static_cast<T>(-3.0))), 2.1, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(-5.1), static_cast<T>(-3.0))), -2.1, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(+0.0), static_cast<T>(1.0))), 0, abs_error);
    EXPECT_NEAR(static_cast<double>(f(static_cast<T>(-0.0), static_cast<T>(1.0))), -0, abs_error);
    // Typically, divisor is compile time constant for which the following aren't as important
    //EXPECT_NEAR(static_cast<double>(f(static_cast<T>(5.1), std::numeric_limits<T>::infinity())), 5.1, abs_error);
    //EXPECT_TRUE(std::isnan(static_cast<double>(f(static_cast<T>(+5.1), static_cast<T>(0.0)))));
}

TEST(Math, ModuloViaFmod)
{
    {
        SCOPED_TRACE("For float");
        auto f = ModuloViaFmod<float>;
        TestModuloFunction<float>(f);
    }
    {
        SCOPED_TRACE("For double");
        auto f = ModuloViaFmod<double>;
        TestModuloFunction<double>(f);
    }
}

TEST(Math, ModuloViaTrunc)
{
    {
        SCOPED_TRACE("For float");
        auto f = ModuloViaTrunc<float>;
        TestModuloFunction<float>(f);
    }
    {
        SCOPED_TRACE("For double");
        auto f = ModuloViaTrunc<double>;
        TestModuloFunction<double>(f);
    }
}

TEST(Math, GetFwdRotationalAngle)
{
    EXPECT_EQ(GetFwdRotationalAngle(0_deg, 0_deg), 0_deg);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle(0_deg, 10_deg) / 1_deg}), -350.0, 0.0001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle(-10_deg, 0_deg) / 1_deg}), -350.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetFwdRotationalAngle(90_deg, -90_deg)/1_deg}), -180.0, 0.0001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle(100_deg, 110_deg) / 1_deg}), -350.0, 0.0001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle( 10_deg,   0_deg) / 1_deg}),  -10.0, 0.0001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle( -2_deg,  +3_deg) / 1_deg}), -355.0, 0.001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle( +2_deg,  -3_deg) / 1_deg}),   -5.0, 0.001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle(-13_deg,  -3_deg) / 1_deg}), -350.0, 0.001);
    EXPECT_NEAR(double(Real{GetFwdRotationalAngle(-10_deg, -20_deg) / 1_deg}),  -10.0, 0.001);
}

TEST(Math, GetRevRotationalAngle)
{
    EXPECT_EQ(GetRevRotationalAngle(0_deg, 0_deg), 0_deg);
    EXPECT_EQ(GetRevRotationalAngle(0_deg, 10_deg), 10_deg);
    // GetRevRotationalAngle(100 * Degree, 110 * Degree) almost equals 10 * Degree (but not exactly)
    EXPECT_EQ(GetRevRotationalAngle(-10_deg, 0_deg), 10_deg);
    EXPECT_NEAR(static_cast<double>(Real{GetRevRotationalAngle(90_deg, -90_deg)/1_deg}), 180.0, 0.0001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle(100_deg, 110_deg) / 1_deg}),  10.0, 0.0001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle( 10_deg,   0_deg) / 1_deg}), 350.0, 0.0001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle( -2_deg,  +3_deg) / 1_deg}),   5.0, 0.001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle( +2_deg,  -3_deg) / 1_deg}), 355.0, 0.001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle(-13_deg,  -3_deg) / 1_deg}),  10.0, 0.001);
    EXPECT_NEAR(double(Real{GetRevRotationalAngle(-10_deg, -20_deg) / 1_deg}), 350.0, 0.001);
}

TEST(Math, Normalize)
{
    const auto v0 = Real(2);
    const auto v1 = Real(2);
    auto value = Vec2{v0, v1};
    const auto length = GetMagnitude(value);
    const auto invLength = Real{1} / length;
    auto magnitude = Real(0);
    EXPECT_NO_THROW(magnitude = Normalize(value));
    EXPECT_EQ(magnitude, length);
    EXPECT_EQ(value[0], value[1]);
    EXPECT_EQ(value[0], v0 * invLength);
    EXPECT_EQ(value[1], v1 * invLength);
}

TEST(Math, GetNormalized)
{
    // Confirm that GetNormalized returns a half-open interval value that's [-Pi, +Pi)...
    // I.e. greater than or equal to -Pi and less than +Pi.
    EXPECT_EQ(GetNormalized(0_deg) / Degree, Real(0));
    EXPECT_DOUBLE_EQ(double(Real(GetNormalized(  0.0_deg) / Degree)),    0.0);
    EXPECT_DOUBLE_EQ(double(Real(GetNormalized(360.0_deg) / Degree)),    0.0);
    EXPECT_DOUBLE_EQ(double(Real(GetNormalized(Pi * 2_rad) / 1_rad)),    0.0);
    EXPECT_DOUBLE_EQ(double(Real(GetNormalized(720.0_deg) / Degree)),    0.0);
    EXPECT_DOUBLE_EQ(double(Real(GetNormalized(Pi * 4_rad) / 1_rad)),    0.0);
    EXPECT_NEAR(double(Real(GetNormalized(   21.3_deg) / Degree)),   21.3, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(   90.0_deg) / Degree)),   90.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(   93.2_deg) / Degree)),   93.2, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  180.0_deg) / Degree)), -180.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  185.4_deg) / Degree)), -174.6, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  190.0_deg) / Degree)), -170.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized( -180.0_deg) / Degree)), -180.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  270.0_deg) / Degree)),  -90.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  395.0_deg) / Degree)),   35.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  396.4_deg) / Degree)),   36.4, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  733.0_deg) / Degree)),   13.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  734.5_deg) / Degree)),   14.5, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  -45.0_deg) / Degree)),  -45.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(  -90.0_deg) / Degree)),  -90.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(-3610.0_deg) / Degree)),  -10.0, 0.01);
    EXPECT_NEAR(double(Real(GetNormalized(-3611.2_deg) / Degree)),  -11.2, 0.01);
    //EXPECT_TRUE(std::isnan(double(Real(GetNormalized(std::numeric_limits<Angle>::infinity()) / Degree))));
    //EXPECT_TRUE(std::isnan(float(Real(GetNormalized(std::numeric_limits<Angle>::infinity()) / Degree))));
    EXPECT_TRUE(std::isnan(float(Real(GetNormalized(std::numeric_limits<Angle>::quiet_NaN()) / Degree))));

    // Following test doesn't work when Real=long double, presumably because of rounding issues.
    //EXPECT_NEAR(double(Real(GetNormalized(Angle{ Real{ 360.0} * Degree}) / Degree)),   0.0, 0.0001);
    EXPECT_NEAR(double(Real(GetNormalized(Angle{ 2 * Pi * 1_rad}) / 1_rad)),   0.0, 0.0001);
    // Following test doesn't work when Real=long double, presumably because of rounding issues.
    //EXPECT_NEAR(double(Real(GetNormalized(Angle{ Real( 720.0) * Degree}) / Degree)),   0.0, 0.0001);
    EXPECT_NEAR(double(Real(GetNormalized(Angle{ 4 * Pi * 1_rad}) / 1_rad)),   0.0, 0.0001);

    if constexpr (std::is_same_v<Real,float>) {
        // Pick an absolute error allowable...
        // Using EXPECT_EQ(a, b) checks exact equality but won't report the exact values.
        // Using EXPECT_NEAR(a, b, abs_err) checks that difference is less than abs_err and reports
        //   the exact values when they're difference is not less than or equal to abs_err.
        // So use EXPECT_NEAR with an abs_err that's less tolerant than 1 ULP of a double at Pi
        // so that code checks for exact equality and reports exact values when they don't match.
        constexpr auto abs_err = 1e-20;

        // Recognize some hex to decimal equavalents to help make sense of the following code.
        EXPECT_NEAR(+0x1.921fb5p+1, +3.1415926218032837, abs_err);
        EXPECT_NEAR(+0x1.921fb6p+1, +3.1415927410125732, abs_err);
        EXPECT_NEAR(+0x1.921fb6p+1, +Pi, abs_err);
        EXPECT_NEAR(+0x1.921fb7p+1, +3.1415928602218628, abs_err);

        EXPECT_NEAR(StripUnit(GetNormalized(Real{+0x1.921fb5p+1f} * 1_rad)),
                    Real{+0x1.921fb5p+1f}, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real{+0x1.921fb6p+1f} * 1_rad)),
                    Real{-0x1.921fb6p+1f}, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real{+0x1.921fb7p+1f} * 1_rad)),
                    Real{-0x1.921fb5p+1f}, abs_err);

        EXPECT_NEAR(StripUnit(GetNormalized(Real{-0x1.921fb5p+1f} * 1_rad)),
                    Real{-0x1.921fb5p+1f}, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real{-0x1.921fb6p+1f} * 1_rad)),
                    Real{-0x1.921fb6p+1f}, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real{-0x1.921fb7p+1f} * 1_rad)),
                    Real{+0x1.921fb5p+1f}, abs_err);
    }
    else if constexpr (std::is_same_v<Real, double>) {
        // Pick an absolute error allowable...
        // Using EXPECT_EQ(a, b) checks exact equality but won't report the exact values.
        // Using EXPECT_NEAR(a, b, abs_err) checks that difference is less than abs_err and reports
        //   the exact values when they're difference is not less than or equal to abs_err.
        // So use EXPECT_NEAR with an abs_err that's less tolerant than 1 ULP of a double at Pi
        // so that code checks for exact equality and reports exact values when they don't match.
        constexpr auto abs_err = 1e-20;

        // Recognize some hex to decimal equavalents to help make sense of the following code.
        EXPECT_EQ(+0x1.921fb54442d13p+1, +3.1415926535897909);
        EXPECT_EQ(+0x1.921fb54442d14p+1, +3.1415926535897913);
        EXPECT_EQ(+0x1.921fb54442d15p+1, +3.1415926535897918);
        EXPECT_EQ(+0x1.921fb54442d16p+1, +3.1415926535897922);
        EXPECT_EQ(+0x1.921fb54442d17p+1, +3.1415926535897927);
        EXPECT_EQ(+0x1.921fb54442d18p+1, +3.1415926535897931);
        EXPECT_EQ(+0x1.921fb54442d18p+1, +Pi);
        EXPECT_EQ(-0x1.921fb54442d13p+1, -3.1415926535897909);
        EXPECT_EQ(-0x1.921fb54442d14p+1, -3.1415926535897913);
        EXPECT_EQ(-0x1.921fb54442d15p+1, -3.1415926535897918);
        EXPECT_EQ(-0x1.921fb54442d16p+1, -3.1415926535897922);
        EXPECT_EQ(-0x1.921fb54442d17p+1, -3.1415926535897927);
        EXPECT_EQ(-0x1.921fb54442d18p+1, -3.1415926535897931);
        EXPECT_EQ(-0x1.921fb54442d18p+1, -Pi);

        // Check that GetNormalized(-Pi) == -Pi
        EXPECT_NEAR(static_cast<double>(Real{GetNormalized(-Pi * 1_rad)/1_rad}),
                    -Pi, abs_err);

        // Check that GetNormalized(-Pi) == GetNormalized(+Pi)...
        EXPECT_NEAR(static_cast<double>(Real{GetNormalized(+Pi * 1_rad)/1_rad}),
                    static_cast<double>(Real{GetNormalized(-Pi * 1_rad)/1_rad}),
                    abs_err);

        // Turning counter-clockwise, check before, during, and after positive Pi...
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d13p+1) * 1_rad)),
                    +0x1.921fb54442d13p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d14p+1) * 1_rad)),
                    +0x1.921fb54442d14p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d15p+1) * 1_rad)),
                    +0x1.921fb54442d15p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d16p+1) * 1_rad)),
                    +0x1.921fb54442d16p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d17p+1) * 1_rad)),
                    +0x1.921fb54442d17p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d18p+1) * 1_rad)),
                    -0x1.921fb54442d18p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d19p+1) * 1_rad)),
                    -0x1.921fb54442d17p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d1ap+1) * 1_rad)),
                    -0x1.921fb54442d16p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d1bp+1) * 1_rad)),
                    -0x1.921fb54442d15p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(+0x1.921fb54442d1cp+1) * 1_rad)),
                    -0x1.921fb54442d14p+1, abs_err);

        // Turning clockwise, check before, during, and after negative Pi...
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d16p+1) * 1_rad)),
                    -0x1.921fb54442d16p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d17p+1) * 1_rad)),
                    -0x1.921fb54442d17p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d18p+1) * 1_rad)),
                    -0x1.921fb54442d18p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d19p+1) * 1_rad)),
                    +0x1.921fb54442d17p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d1ap+1) * 1_rad)),
                    +0x1.921fb54442d16p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d1bp+1) * 1_rad)),
                    +0x1.921fb54442d15p+1, abs_err);
        EXPECT_NEAR(StripUnit(GetNormalized(Real(-0x1.921fb54442d1cp+1) * 1_rad)),
                    +0x1.921fb54442d14p+1, abs_err);
    }

    // Confirm that GetNormalized is similar to atan2(sin(a), cos(a))
    for (auto i = -360; i < +360; ++i) {
        if (i == -180 /*Real=float*/ || i == +180 /*Real=double*/) {
            continue; // skip -Pi and +Pi
        }
        std::ostringstream os;
        const auto angle = i * 1_deg;
        os << "At " << i << " deg, " << double(Real{angle/1_rad}) << "rad: " << std::hexfloat << double(Real{angle/1_rad});
        SCOPED_TRACE(os.str());
        EXPECT_NEAR(double(StripUnit(GetNormalized(angle))), //
                    double(StripUnit(atan2(sin(angle), cos(angle)))), 0.001);
    }
}

//#define PLAYRHO_RUN_EVEN_SUPER_LONG_TESTS 1
#if PLAYRHO_RUN_EVEN_SUPER_LONG_TESTS
TEST(Math, GetNormalizedLong)
{
    auto first = float(-Pi);
    for (auto i = 0; i < 2; ++i) {
        first = nextafter(first, float(-2 * Pi));
    }
    auto last = float(Pi);
    for (auto i = 0; i < 2; ++i) {
        last = nextafter(last, float(+2 * Pi));
    }
    while (first < last) {
        const auto angle = GetNormalized(first * 1_rad);
        EXPECT_EQ(angle, GetNormalized(angle));
        first = nextafter(first, last);
    }
}
#endif

TEST(Math, GetShortestDelta)
{
    EXPECT_EQ(GetShortestDelta(0_deg, 0_deg), 0_deg);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(0_deg, 10_deg) / Degree}), 10.0, 0.01);
    // GetShortestDelta(100 * Degree, 110 * Degree) almost equals 10 * Degree (but not exactly)
    EXPECT_NEAR(double(Real{GetShortestDelta(100_deg, 110_deg) / Degree}), 10.0, 0.0001);
    EXPECT_NEAR(double(Real{GetShortestDelta(10_deg, 0_deg) / Degree}), -10.0, 0.0001);
    EXPECT_NEAR(double(Real{GetShortestDelta(-10_deg, 0_deg) / Degree}), 10.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(+90_deg, -89_deg)/1_deg}), -179.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(+89_deg, -90_deg)/1_deg}), -179.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(+80_deg, -80_deg)/1_deg}), -160.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(-90_deg, +89_deg)/1_deg}), +179.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(-89_deg, +90_deg)/1_deg}), +179.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(-80_deg, +80_deg)/1_deg}), +160.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(+179_deg, -179_deg)/1_deg}), +2.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(+179_deg, -179_deg - 360_deg)/1_deg}), +2.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(-179_deg, +179_deg)/1_deg}), -2.0, 0.0001);
    EXPECT_NEAR(static_cast<double>(Real{GetShortestDelta(-179_deg, +179_deg + 360_deg)/1_deg}), -2.0, 0.0001);
    EXPECT_NEAR(double(Real{GetShortestDelta(-Pi * Radian, +Pi * Radian) / Degree}), 0.0, 0.001);
    EXPECT_NEAR(double(Real{GetShortestDelta(+Pi * Radian, -Pi * Radian) / Degree}), 0.0, 0.001);
    EXPECT_NEAR(double(Real{GetShortestDelta(-2_deg, +3_deg) / Degree}), 5.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(+2_deg, -3_deg) / Degree}), -5.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(-13_deg, -3_deg) / Degree}), 10.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(-10_deg, -20_deg) / Degree}), -10.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(10_deg, 340_deg) / Degree}), -30.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(400_deg, 440_deg) / Degree}), 40.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(400_deg, 300_deg) / Degree}), -100.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(400_deg, 100_deg) / Degree}), 60.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(800_deg, 100_deg) / Degree}), 20.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(400_deg, -100_deg) / Degree}), -140.0, 0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(-400_deg, 10_deg) / Degree}), 50.0, 0.01);
    {
        const auto a0 = std::nextafter(4.0f, 5.0f) * 1_deg;
        const auto a1 = 4.0f * 1_deg;
        const auto diff = double(Real{(a1 - a0) / 1_deg});
        EXPECT_NEAR(double(Real{GetShortestDelta(a0, a1) / 1_deg}), diff, 1e-18);
    }
    {
        const auto a0 = std::nextafter(0.0f, 1.0f) * 1_deg;
        const auto a1 = 0.0f * 1_deg;
        const auto diff = double(Real{(a1 - a0) / 1_deg});
        EXPECT_NEAR(double(Real{GetShortestDelta(a0, a1) / 1_deg}), diff, 1e-18);
    }
    EXPECT_NEAR(double(Real{GetShortestDelta(4.00000_deg, 4.00001_deg) / 1_deg}), +0.00001, 0.000001);
    EXPECT_NEAR(double(Real{GetShortestDelta(+2_deg, -3_deg) / 1_deg}), //
                double(Real{AlternateGetShortestDelta(+2_deg, -3_deg) / 1_deg}), //
                0.01);
    EXPECT_NEAR(double(Real{GetShortestDelta(+179_deg, -179_deg) / 1_deg}), //
                double(Real{AlternateGetShortestDelta(+179_deg, -179_deg) / 1_deg}), //
                0.0001);
    EXPECT_NEAR(double(Real{GetShortestDelta(+179_deg + 360_deg, -179_deg) / 1_deg}), //
                double(Real{AlternateGetShortestDelta(+179_deg + 720_deg, -179_deg) / 1_deg}), //
                0.0001);
}

TEST(Math, GetPositionDoesntFailWithPeculiarBeta)
{
    /*
     * If GetPosition is implemented as: pos0 * (1 - beta) + pos1 * beta.
     * Then it fails the following test when Real is implemented via float.
     * This is due to floating point inaccuracy.
     *
     * If GetPosition is implemented as: pos0 + (pos1 - pos0) * beta.
     * Then it passes the following test when Real is implemented via float.
     */

    const auto x = Real{2.587699890136719e-02f};
    const auto y = Real{5.515012264251709e+00f};
    const auto value = Real{0.0866042823f};

    const auto oldPos = Position{Vec2{x, y} * Meter, 0_rad};
    const auto newPos = GetPosition(oldPos, oldPos, value);
    
    EXPECT_EQ(oldPos.linear, newPos.linear);
    EXPECT_EQ(oldPos.angular, newPos.angular);
}

TEST(Math, GetPosition)
{
    EXPECT_EQ(GetPosition(Position{}, Position{}, Real(0.0)), (Position{}));
    EXPECT_EQ(GetPosition(Position{}, Position{Length2{2_m, 2_m}, 2_rad}, Real(0.0)),
              (Position{Length2{0_m, 0_m}, 0_rad}));
    EXPECT_EQ(GetPosition(Position{}, Position{Length2{2_m, 2_m}, 2_rad}, Real(0.5)),
              (Position{Length2{1_m, 1_m}, 1_rad}));
    EXPECT_EQ(GetPosition(Position{}, Position{Length2{2_m, 2_m}, 2_rad}, Real(1.0)),
              (Position{Length2{2_m, 2_m}, 2_rad}));

#if 1
    // Test a case that's maybe less obvious...
    // See https://github.com/louis-langholtz/PlayRho/issues/331#issuecomment-507412550
    const auto p0 = Position{Length2{-0.1615_m, -10.2494_m}, -3.1354_rad};
    const auto p1 = Position{Length2{-0.3850_m, -10.1851_m}, +3.1258_rad};
    const auto p = GetPosition(p0, p1, Real(0.2580));
    constexpr auto abserr = 0.000001;
    EXPECT_NEAR(static_cast<double>(Real(GetX(p.linear) / 1_m)), -0.21916300, abserr);
    EXPECT_NEAR(static_cast<double>(Real(GetY(p.linear) / 1_m)), -10.232810974121094, abserr);
    EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -1.52001, abserr);
#else
    {
        // Test a case that's maybe less obvious...
        // See https://github.com/louis-langholtz/PlayRho/issues/331#issuecomment-507412550
        const auto p0 = Position{Length2{-0.1615_m, -10.2494_m}, -3.1354_rad};
        const auto p1 = Position{Length2{-0.3850_m, -10.1851_m}, +3.1258_rad};
        const auto p = GetPosition(p0, p1, Real(0.2580));
        constexpr auto abserr = 0.000001;
        EXPECT_NEAR(static_cast<double>(Real(GetX(p.linear) / 1_m)), -0.21916300, abserr);
        EXPECT_NEAR(static_cast<double>(Real(GetY(p.linear) / 1_m)), -10.232810974121094, abserr);
        //EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -1.52001, abserr);
        EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -3.1410722732543945, abserr);
    }
    {
        // Test a case that's maybe less obvious...
        // See https://github.com/louis-langholtz/PlayRho/issues/331#issuecomment-507412550
        const auto p0 = Position{Length2{-0.1615_m, -10.2494_m}, -3.1354_rad + Pi * 10_rad};
        const auto p1 = Position{Length2{-0.3850_m, -10.1851_m}, +3.1258_rad};
        const auto p = GetPosition(p0, p1, Real(0.2580));
        constexpr auto abserr = 0.000001;
        EXPECT_NEAR(static_cast<double>(Real(GetX(p.linear) / 1_m)), -0.21916300, abserr);
        EXPECT_NEAR(static_cast<double>(Real(GetY(p.linear) / 1_m)), -10.232810974121094, abserr);
        //EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -1.52001, abserr);
        EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -3.1410722732543945, abserr);
    }
    {
        // Test a case that's maybe less obvious...
        // See https://github.com/louis-langholtz/PlayRho/issues/331#issuecomment-507412550
        const auto p0 = Position{Length2{-0.1615_m, -10.2494_m}, -3.1354_rad + Pi * 10_rad};
        const auto p1 = Position{Length2{-0.3850_m, -10.1851_m}, +3.1258_rad + Pi * 4_rad};
        const auto p = GetPosition(p0, p1, Real(0.2580));
        constexpr auto abserr = 0.000001;
        EXPECT_NEAR(static_cast<double>(Real(GetX(p.linear) / 1_m)), -0.21916300, abserr);
        EXPECT_NEAR(static_cast<double>(Real(GetY(p.linear) / 1_m)), -10.232810974121094, abserr);
        //EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -1.52001, abserr);
        EXPECT_NEAR(static_cast<double>(Real(p.angular / 1_rad)), -3.1410722732543945, abserr);
    }
#endif
}

TEST(Math, CapPosition)
{
    EXPECT_EQ(GetX(Cap(Position{}, ConstraintSolverConf{}).linear), 0_m);
    EXPECT_EQ(GetY(Cap(Position{}, ConstraintSolverConf{}).linear), 0_m);
    EXPECT_EQ(Cap(Position{}, ConstraintSolverConf{}).angular, 0_deg);

    EXPECT_NEAR(double(StripUnit(GetX(Cap(Position{Length2{10_m, 0_m}, 360_deg}, ConstraintSolverConf{}).linear))),
                double(StripUnit(ConstraintSolverConf{}.maxLinearCorrection)), 0.0001);
    EXPECT_NEAR(double(StripUnit(GetY(Cap(Position{Length2{0_m, 10_m}, 360_deg}, ConstraintSolverConf{}).linear))),
                double(StripUnit(ConstraintSolverConf{}.maxLinearCorrection)), 0.0001);
    EXPECT_NEAR(double(StripUnit(Cap(Position{Length2{0_m, 0_m}, 360_deg}, ConstraintSolverConf{}).angular)),
                double(StripUnit(ConstraintSolverConf{}.maxAngularCorrection)), 0.0001);
}

TEST(Math, ToiTolerance)
{
    // What is the max vr for which the following still holds true?
    //   vr + DefaultLinearSlop / 4 > vr
    // The max vr for which (nextafter(vr, MaxFloat) - vr) <= DefaultLinearSlop / 4.
    // I.e. the max vr for which (nextafter(vr, MaxFloat) - vr) <= 0.000025

    const auto linearSlop = 0.0001f;
    const auto tolerance = linearSlop / 4;
    {
        const auto vr = 511.0f;
        EXPECT_GT(vr + tolerance, vr);
    }
    {
        const auto vr = 512.0f;
        EXPECT_EQ(vr + tolerance, vr);
    }
}

struct Coords {
    float x, y;
};

TEST(Math, LengthFasterThanHypot)
{
    constexpr auto iterations = unsigned(5000000);
    
    std::chrono::duration<double> elapsed_secs_length;
    std::chrono::duration<double> elapsed_secs_hypot;
    
    const auto v1 = Coords{10.8f, 99.02f};
    const auto v2 = Coords{-6.01f, 31.2f};
    const auto v3 = Coords{409183.2f, 0.00023f};
    const auto v4 = Coords{-0.004f, 0.001f};
    const auto v5 = Coords{-432.1f, -9121.0f};
    const auto v6 = Coords{32.1f, -21.0f};
    const auto v7 = Coords{12088.032f, 7612.823f};
    const auto v8 = Coords{7612.823f, -7612.823f};

    auto totalLength = 0.0f;
    auto totalHypot = 0.0f;

    {
        // Time the "length" algorithm: sqrt(x^2 + y^2).
        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        start = std::chrono::high_resolution_clock::now();
        for (auto i = decltype(iterations){0}; i < iterations; ++i)
        {
            const auto l1 = std::sqrt(Square(v1.x * i) + Square(v1.y * i));
            const auto l2 = std::sqrt(Square(v2.x * i) + Square(v2.y * i));
            const auto l3 = std::sqrt(Square(v3.x * i) + Square(v3.y * i));
            const auto l4 = std::sqrt(Square(v4.x * i) + Square(v4.y * i));
            const auto l5 = std::sqrt(Square(v5.x * i) + Square(v5.y * i));
            const auto l6 = std::sqrt(Square(v6.x * i) + Square(v6.y * i));
            const auto l7 = std::sqrt(Square(v7.x * i) + Square(v7.y * i));
            const auto l8 = std::sqrt(Square(v8.x * i) + Square(v8.y * i));
            totalLength += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
        }
        end = std::chrono::high_resolution_clock::now();
        elapsed_secs_length = end - start;
    }
    
    {
        // Time the "hypot" algorithm: hypot(x, y).
        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        start = std::chrono::high_resolution_clock::now();
        for (auto i = decltype(iterations){0}; i < iterations; ++i)
        {
            const auto l1 = std::hypot(v1.x * i, v1.y * i);
            const auto l2 = std::hypot(v2.x * i, v2.y * i);
            const auto l3 = std::hypot(v3.x * i, v3.y * i);
            const auto l4 = std::hypot(v4.x * i, v4.y * i);
            const auto l5 = std::hypot(v5.x * i, v5.y * i);
            const auto l6 = std::hypot(v6.x * i, v6.y * i);
            const auto l7 = std::hypot(v7.x * i, v7.y * i);
            const auto l8 = std::hypot(v8.x * i, v8.y * i);
            totalHypot += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
        }
        end = std::chrono::high_resolution_clock::now();
        elapsed_secs_hypot = end - start;
    }
    
    EXPECT_LT(elapsed_secs_length.count(), elapsed_secs_hypot.count());
    EXPECT_NEAR(totalLength, totalHypot, totalLength / 10.0);
}

TEST(Math, GetCircleVertices)
{
    {
        const auto vertices = GetCircleVertices(0_m, 0);
        EXPECT_EQ(vertices, std::vector<Length2>());
    }
    {
        const auto vertices = GetCircleVertices(0_m, 1);
        EXPECT_EQ(vertices, std::vector<Length2>({Length2{}, Length2{}}));
    }
    {
        const auto vertices = GetCircleVertices(0_m, 2);
        EXPECT_EQ(vertices, std::vector<Length2>({Length2{}, Length2{}, Length2{}}));
    }
    {
        const auto vertices = GetCircleVertices(0_m, 3);
        EXPECT_EQ(vertices, std::vector<Length2>({Length2{}, Length2{}, Length2{}, Length2{}}));
    }
    {
        const auto vertices = GetCircleVertices(1_m, 0);
        EXPECT_EQ(vertices, std::vector<Length2>());
    }
    {
        const auto vertices = GetCircleVertices(1_m, 1);
        EXPECT_EQ(vertices, std::vector<Length2>({Length2(1_m, 0_m), Length2(1_m, 0_m)}));
    }
    {
        const auto vertices = GetCircleVertices(1_m, 2);
        EXPECT_EQ(vertices[0], Length2(1_m, 0_m));
        EXPECT_NEAR(static_cast<double>(Real(GetX(vertices[1]) / Meter)), -1.0, 0.0001);
        EXPECT_NEAR(static_cast<double>(Real(GetY(vertices[1]) / Meter)),  0.0, 0.0001);
        EXPECT_EQ(vertices[2], Length2(1_m, 0_m));
    }
}

TEST(Math, AlmostZero)
{
    EXPECT_TRUE(AlmostZero(0.0f));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0f, +1.0f)));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0f, -1.0f)));
    EXPECT_TRUE(AlmostZero(std::nextafter(std::numeric_limits<float>::min(), 0.0f)));
    EXPECT_FALSE(AlmostZero(std::numeric_limits<float>::min()));
    EXPECT_FALSE(AlmostZero(+1.0f));
    EXPECT_FALSE(AlmostZero(-1.0f));

    EXPECT_TRUE(AlmostZero(0.0));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0, +1.0)));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0, -1.0)));
    EXPECT_TRUE(AlmostZero(std::nextafter(std::numeric_limits<double>::min(), 0.0)));
    EXPECT_FALSE(AlmostZero(std::numeric_limits<double>::min()));
    EXPECT_FALSE(AlmostZero(+1.0));
    EXPECT_FALSE(AlmostZero(-1.0));

    EXPECT_TRUE(AlmostZero(0.0l));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0l, +1.0l)));
    EXPECT_TRUE(AlmostZero(std::nextafter(0.0l, -1.0l)));
    EXPECT_TRUE(AlmostZero(std::nextafter(std::numeric_limits<long double>::min(), 0.0l)));
    EXPECT_FALSE(AlmostZero(std::numeric_limits<long double>::min()));
    EXPECT_FALSE(AlmostZero(+1.0l));
    EXPECT_FALSE(AlmostZero(-1.0l));
}

TEST(Math, InvertZeroIsZero)
{
    const auto mat = Mat22{};
    const auto out = Invert(mat);
    EXPECT_EQ(get<0>(get<0>(out)), get<0>(get<0>(mat)));
    EXPECT_EQ(get<0>(get<1>(out)), get<0>(get<1>(mat)));
    EXPECT_EQ(get<1>(get<0>(out)), get<1>(get<0>(mat)));
    EXPECT_EQ(get<1>(get<1>(out)), get<1>(get<1>(mat)));
}

TEST(Math, InvertOneIsZero)
{
    // Use pragmas to quiet MS Visual Studio re: "potential divide by zero".
#pragma warning( push )
#pragma warning( disable: 4723 )
    const auto mat = Mat22{Vec2{Real(1), Real(1)}, Vec2{Real(1), Real(1)}};
    const auto out = Invert(mat);
    EXPECT_EQ(get<0>(get<0>(out)), Real(0));
    EXPECT_EQ(get<0>(get<1>(out)), Real(0));
    EXPECT_EQ(get<1>(get<0>(out)), Real(0));
    EXPECT_EQ(get<1>(get<1>(out)), Real(0));
#pragma warning( pop )
}

TEST(Math, clamp)
{
    // Check lo and hi work as documented on clamp...
    const auto NaN = std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ(std::clamp(-1.0,  0.0, +1.0), 0.0);
    EXPECT_EQ(std::clamp(+1.0, -1.0,  0.0), 0.0);
    EXPECT_EQ(std::clamp(0.0, NaN, NaN), 0.0);
    EXPECT_EQ(std::clamp(8.0, NaN, NaN), 8.0);
    EXPECT_EQ(std::clamp(0.0, -1.0, NaN), 0.0);
    EXPECT_EQ(std::clamp(-2.0, -1.0, NaN), -1.0);
    EXPECT_EQ(std::clamp(0.0, NaN, +1.0), 0.0);
    EXPECT_EQ(std::clamp(2.0, NaN, +1.0), 1.0);
    EXPECT_EQ(std::clamp(0.0, -1.0, +1.0), 0.0);
    EXPECT_TRUE(std::isnan(std::clamp(NaN, -1.0, +1.0)));
    EXPECT_TRUE(std::isnan(std::clamp(NaN, NaN, +1.0)));
    EXPECT_TRUE(std::isnan(std::clamp(NaN, -1.0, NaN)));
    EXPECT_TRUE(std::isnan(std::clamp(NaN, NaN, NaN)));
}

TEST(Math, GetReflectionMatrix)
{
    {
        // reflection against y axis
        const auto m = GetReflectionMatrix(UnitVec::GetRight());
        EXPECT_EQ(m[0][0], Real(-1));
        EXPECT_EQ(m[0][1], Real(0));
        EXPECT_EQ(m[1][0], Real(0));
        EXPECT_EQ(m[1][1], Real(+1));
        const auto vp = m * Vec2{+2, +3};
        EXPECT_EQ(get<0>(vp), -2);
        EXPECT_EQ(get<1>(vp), +3);
    }
    {
        // reflection against y axis
        const auto m = GetReflectionMatrix(UnitVec::GetLeft());
        EXPECT_EQ(m[0][0], Real(-1));
        EXPECT_EQ(m[0][1], Real(0));
        EXPECT_EQ(m[1][0], Real(0));
        EXPECT_EQ(m[1][1], Real(+1));
        const auto vp = m * Vec2{+2, +3};
        EXPECT_EQ(get<0>(vp), -2);
        EXPECT_EQ(get<1>(vp), +3);
    }
    {
        // reflection against x axis
        const auto m = GetReflectionMatrix(UnitVec::GetTop());
        EXPECT_EQ(m[0][0], Real(+1));
        EXPECT_EQ(m[0][1], Real(0));
        EXPECT_EQ(m[1][0], Real(0));
        EXPECT_EQ(m[1][1], Real(-1));
        const auto vp = m * Vec2{+2, +3};
        EXPECT_EQ(get<0>(vp), +2);
        EXPECT_EQ(get<1>(vp), -3);
    }
    {
        // reflection against x axis
        const auto m = GetReflectionMatrix(UnitVec::GetBottom());
        EXPECT_EQ(m[0][0], Real(+1));
        EXPECT_EQ(m[0][1], Real(0));
        EXPECT_EQ(m[1][0], Real(0));
        EXPECT_EQ(m[1][1], Real(-1));
        const auto vp = m * Vec2{+2, +3};
        EXPECT_EQ(get<0>(vp), +2);
        EXPECT_EQ(get<1>(vp), -3);
    }
    {
        const auto m = GetReflectionMatrix(UnitVec::GetTopRight());
        EXPECT_NEAR(static_cast<double>(m[0][0]),  0.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[0][1]), -1.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[1][0]), -1.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[1][1]),  0.0, 0.000001);
        const auto vp = m * Vec2{+2, +3};
        EXPECT_NEAR(static_cast<double>(get<0>(vp)), -3.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(get<1>(vp)), -2.0, 0.000001);
    }
    {
        const auto m = GetReflectionMatrix(UnitVec::GetBottomRight());
        EXPECT_NEAR(static_cast<double>(m[0][0]),  0.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[0][1]), +1.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[1][0]), +1.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(m[1][1]),  0.0, 0.000001);
        const auto vp = m * Vec2{+2, +3};
        EXPECT_NEAR(static_cast<double>(get<0>(vp)), +3.0, 0.000001);
        EXPECT_NEAR(static_cast<double>(get<1>(vp)), +2.0, 0.000001);
    }
}

TEST(Math, ToSigned)
{
    EXPECT_EQ(ToSigned(0), 0);
    EXPECT_EQ(ToSigned(42), 42);
    EXPECT_EQ(ToSigned(-42), -42);
    EXPECT_EQ(ToSigned(42u), 42);
    EXPECT_EQ(ToSigned(static_cast<std::uint8_t>(255u)), -1);
}

TEST(Math, GetModuloNext)
{
    EXPECT_EQ(GetModuloNext(std::uint8_t(0u), std::uint8_t(1u)), std::uint8_t(0u));
    EXPECT_EQ(GetModuloNext(std::uint8_t(0u), std::uint8_t(2u)), std::uint8_t(1u));
    EXPECT_EQ(GetModuloNext(std::uint8_t(254u), std::uint8_t(255u)), std::uint8_t(0u));
    EXPECT_EQ(GetModuloNext(std::int8_t(0), std::int8_t(1)), std::int8_t(0));
    EXPECT_EQ(GetModuloNext(std::int8_t(0), std::int8_t(2)), std::int8_t(1));
    EXPECT_EQ(GetModuloNext(std::int8_t(1), std::int8_t(2)), std::int8_t(0));
    EXPECT_EQ(GetModuloNext(std::int8_t(126), std::int8_t(127)), std::int8_t(0));
}

TEST(Math, IsPowerOfTwo)
{
    EXPECT_TRUE(IsPowerOfTwo(1));
    EXPECT_TRUE(IsPowerOfTwo(2));
    EXPECT_TRUE(IsPowerOfTwo(4));
    EXPECT_TRUE(IsPowerOfTwo(8));

    EXPECT_FALSE(IsPowerOfTwo(0));
    EXPECT_FALSE(IsPowerOfTwo(3));
    EXPECT_FALSE(IsPowerOfTwo(5));
    EXPECT_FALSE(IsPowerOfTwo(6));

    EXPECT_TRUE(IsPowerOfTwo(1u));
    EXPECT_TRUE(IsPowerOfTwo(2u));
    EXPECT_TRUE(IsPowerOfTwo(4u));
    EXPECT_TRUE(IsPowerOfTwo(8u));

    EXPECT_FALSE(IsPowerOfTwo(0u));
    EXPECT_FALSE(IsPowerOfTwo(3u));
    EXPECT_FALSE(IsPowerOfTwo(5u));
    EXPECT_FALSE(IsPowerOfTwo(6u));
}
