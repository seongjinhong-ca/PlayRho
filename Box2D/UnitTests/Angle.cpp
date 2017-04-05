/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/Box2D
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "gtest/gtest.h"
#include <Box2D/Common/Math.hpp>

using namespace box2d;

TEST(Angle, ByteSizeIs_4_8_or_16)
{
	switch (sizeof(RealNum))
	{
		case  4: EXPECT_EQ(sizeof(Angle), size_t(4)); break;
		case  8: EXPECT_EQ(sizeof(Angle), size_t(8)); break;
		case 16: EXPECT_EQ(sizeof(Angle), size_t(16)); break;
		default: FAIL(); break;
	}
}

TEST(Angle, GetRevRotationalAngle)
{
	EXPECT_EQ(GetRevRotationalAngle(RealNum{0} * Degree, RealNum{0} * Degree), RealNum{0} * Degree);
	EXPECT_EQ(GetRevRotationalAngle(RealNum{0} * Degree, 10.0f * Degree), 10.0f * Degree);
	// GetRevRotationalAngle(100 * Degree, 110 * Degree) almost equals 10 * Degree (but not exactly)
	EXPECT_NEAR(double(GetRevRotationalAngle(100.0f * Degree, 110.0f * Degree) / Degree), double(10), 0.0001);
	EXPECT_NEAR(double(GetRevRotationalAngle( 10.0f * Degree, RealNum{0} * Degree) / Degree), double(350), 0.0001);
	EXPECT_EQ(GetRevRotationalAngle(-10.0f * Degree, RealNum{0} * Degree), 10.0f * Degree);
	EXPECT_EQ(GetRevRotationalAngle(90.0f * Degree, -90.0f * Degree), 180.0f * Degree);
}

TEST(Angle, GetNormalized)
{
	EXPECT_EQ(GetNormalized(RealNum{0} * Degree) / Degree, RealNum{0});
	EXPECT_EQ(GetNormalized(90.0f * Degree) / Degree, RealNum{90});
	EXPECT_EQ(GetNormalized(180.0f * Degree) / Degree, RealNum{180});
	EXPECT_NEAR(double(GetNormalized(270.0f * Degree) / Degree), double(270), 0.0002);
	EXPECT_EQ(GetNormalized(360.0f * Degree) / Degree, RealNum{0});
	EXPECT_NEAR(GetNormalized(395.0f * Degree) / Degree, 35.0, 0.0002);
	EXPECT_EQ(GetNormalized(720.0f * Degree) / Degree, RealNum{0});
	EXPECT_NEAR(GetNormalized(733.0f * Degree) / Degree, 13.0f, 0.001);
	EXPECT_EQ(GetNormalized(-45.0f * Degree) / Degree, RealNum{-45});
	EXPECT_EQ(GetNormalized(-90.0f * Degree) / Degree, RealNum{-90});
	EXPECT_NEAR(GetNormalized(-3610.0f * Degree) / Degree, -10.0, 0.001);
}
