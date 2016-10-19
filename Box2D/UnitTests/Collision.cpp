/*
 * Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
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
#include <Box2D/Collision/CollideShapes.hpp>
#include <Box2D/Collision/Manifold.hpp>
#include <Box2D/Collision/WorldManifold.hpp>
#include <Box2D/Collision/Shapes/CircleShape.h>
#include <Box2D/Collision/Shapes/PolygonShape.h>

using namespace box2d;

TEST(CollideShapes, CircleCircleOrientedHorizontally)
{
	const auto r1 = float_t(1);
	const auto r2 = float_t(1);
	const auto s1 = CircleShape{r1};
	const auto s2 = CircleShape{r2};
	const auto p1 = Vec2{11, -4};
	const auto p2 = Vec2{13, -4};
	const auto t1 = Transformation{p1, Rot_identity};
	const auto t2 = Transformation{p2, Rot_identity};
	
	// put shape 1 to left of shape 2
	const auto manifold = CollideShapes(s1, t1, s2, t2);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_circles);
	
	EXPECT_FALSE(IsValid(manifold.GetLocalNormal()));
	EXPECT_EQ(manifold.GetLocalPoint(), s1.GetPosition());
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(1));

	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, s2.GetPosition());
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 0);
}

TEST(CollideShapes, CircleCircleOrientedVertically)
{
	const auto r1 = float_t(1);
	const auto r2 = float_t(1);
	const auto s1 = CircleShape{r1};
	const auto s2 = CircleShape{r2};
	const auto p1 = Vec2{7, -2};
	const auto p2 = Vec2{7, -1};
	
	// Rotations don't matter so long as circle shapes' centers are at (0, 0).
	const auto t1 = Transformation{p1, Rot(DegreesToRadians(45))};
	const auto t2 = Transformation{p2, Rot(DegreesToRadians(-21))};
	
	// put shape 1 to left of shape 2
	const auto manifold = CollideShapes(s1, t1, s2, t2);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_circles);
	
	EXPECT_FALSE(IsValid(manifold.GetLocalNormal()));
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(0, 0));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(1));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(0, 0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 0);
}

TEST(CollideShapes, TallRectangleLeftCircleRight)
{
	const auto r2 = float_t(1);
	const auto hx = float_t(2.2);
	const auto hy = float_t(4.8);

	const auto s1 = PolygonShape(hx, hy);
	ASSERT_EQ(s1.GetVertex(0), Vec2(+hx, -hy)); // bottom right
	ASSERT_EQ(s1.GetVertex(1), Vec2(+hx, +hy)); // top right
	ASSERT_EQ(s1.GetVertex(2), Vec2(-hx, +hy)); // top left
	ASSERT_EQ(s1.GetVertex(3), Vec2(-hx, -hy)); // bottom left

	const auto s2 = CircleShape{r2};
	
	const auto p1 = Vec2{-1, 0};
	const auto p2 = Vec2{3, 0};
	const auto t1 = Transformation{p1, Rot{DegreesToRadians(45)}};
	const auto t2 = Transformation{p2, Rot{0}};
	
	// rotate rectangle 45 degrees and put it on the left of the circle
	const auto manifold = CollideShapes(s1, t1, s2, t2);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);
	
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(1, 0));
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(hx, 0));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(1));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(0, 0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 0);
}

TEST(CollideShapes, IdenticalOverlappingSquares)
{
	const auto dim = float_t(2);
	const auto shape = PolygonShape(dim, dim);
	ASSERT_EQ(shape.GetVertex(0), Vec2(+2, -2)); // bottom right
	ASSERT_EQ(shape.GetVertex(1), Vec2(+2, +2)); // top right
	ASSERT_EQ(shape.GetVertex(2), Vec2(-2, +2)); // top left
	ASSERT_EQ(shape.GetVertex(3), Vec2(-2, -2)); // bottom left

	const auto xfm = Transformation(Vec2_zero, Rot{0});
	const auto manifold = CollideShapes(shape, xfm, shape, xfm);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);

	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(+1, 0));
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(+2, 0));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(2));

	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(-2, +2)); // top left
	EXPECT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 2);

	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_EQ(manifold.GetPoint(1).localPoint, Vec2(-2, -2)); // bottom left
	EXPECT_EQ(manifold.GetPoint(1).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexB, 3);
}

TEST(CollideShapes, IdenticalVerticalTouchingSquares)
{
	const auto dim = float_t(2);
	const auto shape = PolygonShape(dim, dim);
	ASSERT_EQ(shape.GetVertex(0), Vec2(+2, -2)); // bottom right
	ASSERT_EQ(shape.GetVertex(1), Vec2(+2, +2)); // top right
	ASSERT_EQ(shape.GetVertex(2), Vec2(-2, +2)); // top left
	ASSERT_EQ(shape.GetVertex(3), Vec2(-2, -2)); // bottom left

	const auto xfm0 = Transformation(Vec2{0, -1}, Rot{0}); // bottom
	const auto xfm1 = Transformation(Vec2{0, +1}, Rot{0}); // top
	const auto manifold = CollideShapes(shape, xfm0, shape, xfm1);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);
	
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(0,+2));	
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(0,+1));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(2));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(-2, -2)); // bottom left
	EXPECT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 1);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 3);
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_EQ(manifold.GetPoint(1).localPoint, Vec2(+2, -2)); // bottom right
	EXPECT_EQ(manifold.GetPoint(1).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexA, 1);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexB, 0);
}

TEST(CollideShapes, IdenticalHorizontalTouchingSquares)
{
	const auto dim = float_t(2);
	const auto shape = PolygonShape(dim, dim);
	ASSERT_EQ(shape.GetVertex(0), Vec2(+2, -2)); // bottom right
	ASSERT_EQ(shape.GetVertex(1), Vec2(+2, +2)); // top right
	ASSERT_EQ(shape.GetVertex(2), Vec2(-2, +2)); // top left
	ASSERT_EQ(shape.GetVertex(3), Vec2(-2, -2)); // bottom left

	const auto xfm0 = Transformation(Vec2{-2, 0}, Rot{0}); // left
	const auto xfm1 = Transformation(Vec2{+2, 0}, Rot{0}); // right
	const auto manifold = CollideShapes(shape, xfm0, shape, xfm1);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);
	
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(+2, 0));	
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(+1, 0));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(2));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(-2, +2)); // top left
	EXPECT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 2);
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_EQ(manifold.GetPoint(1).localPoint, Vec2(-2, -2)); // bottom left
	EXPECT_EQ(manifold.GetPoint(1).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexB, 3);
}

TEST(CollideShapes, SquareCornerUnderSquareFace)
{
	const auto dim = float_t(2);

	// creates a square
	const auto shape = PolygonShape(dim, dim);
	ASSERT_EQ(shape.GetVertex(0), Vec2(+2, -2)); // bottom right
	ASSERT_EQ(shape.GetVertex(1), Vec2(+2, +2)); // top right
	ASSERT_EQ(shape.GetVertex(2), Vec2(-2, +2)); // top left
	ASSERT_EQ(shape.GetVertex(3), Vec2(-2, -2)); // bottom left
	
	const auto rot0 = Rot{DegreesToRadians(45)};
	const auto rot1 = Rot{0};
	const auto xfm0 = Transformation(Vec2{0, -2}, rot0); // bottom
	const auto xfm1 = Transformation(Vec2{0, +2}, rot1); // top
	
	// Rotate square A and put it below square B
	const auto manifold = CollideShapes(shape, xfm0, shape, xfm1);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceB);
	
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(0, -1));
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(0, -2));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(1));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	
	// localPoint is almost equal to Vec2(2, 2) but it's not exactly equal.
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).localPoint.x, float_t(+2)); // top right
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).localPoint.y, float_t(+2)); // top right
	
	EXPECT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 1);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 3);
	
	// Also check things in terms of world coordinates...
	const auto world_manifold = GetWorldManifold(manifold, xfm0, float_t(0), xfm1, float_t(0));
	EXPECT_EQ(world_manifold.GetPointCount(), manifold.GetPointCount());
	
	EXPECT_EQ(world_manifold.GetNormal(), Vec2(0, +1));
	
	const auto corner_point = Rotate(Vec2{dim, dim}, rot0) + xfm0.p;
	EXPECT_FLOAT_EQ(corner_point.x, float_t(0));
	EXPECT_FLOAT_EQ(corner_point.y, float_t(0.82842684));
	
	ASSERT_GT(world_manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).x, corner_point.x / 2);
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).y, corner_point.y / 2);
	EXPECT_FLOAT_EQ(world_manifold.GetSeparation(0), -corner_point.y);
}

TEST(CollideShapes, HorizontalOverlappingRects1)
{
	// square
	const auto shape0 = PolygonShape(2, 2);
	ASSERT_EQ(shape0.GetVertex(0), Vec2(+2,-2)); // bottom right
	ASSERT_EQ(shape0.GetVertex(1), Vec2(+2,+2)); // top right
	ASSERT_EQ(shape0.GetVertex(2), Vec2(-2,+2)); // top left
	ASSERT_EQ(shape0.GetVertex(3), Vec2(-2,-2)); // bottom left
	
	// wide rectangle
	const auto shape1 = PolygonShape(3, 1.5);
	ASSERT_EQ(shape1.GetVertex(0), Vec2(float_t(+3.0), float_t(-1.5))); // bottom right
	ASSERT_EQ(shape1.GetVertex(1), Vec2(float_t(+3.0), float_t(+1.5))); // top right
	ASSERT_EQ(shape1.GetVertex(2), Vec2(float_t(-3.0), float_t(+1.5))); // top left
	ASSERT_EQ(shape1.GetVertex(3), Vec2(float_t(-3.0), float_t(-1.5))); // bottom left

	const auto xfm0 = Transformation(Vec2{-2, 0}, Rot{0}); // left
	const auto xfm1 = Transformation(Vec2{+2, 0}, Rot{0}); // right
	
	// put square left, wide rectangle right
	const auto manifold = CollideShapes(shape0, xfm0, shape1, xfm1);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);
	
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(float_t(+2), float_t(0)));
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(float_t(+1), float_t(0)));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(2));
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_EQ(manifold.GetPoint(0).localPoint, Vec2(float_t(-3.0), float_t(+1.5))); // top left
	EXPECT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 2);
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_EQ(manifold.GetPoint(1).localPoint, Vec2(float_t(-3.0), float_t(-1.5))); // bottom left
	EXPECT_EQ(manifold.GetPoint(1).normalImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeA, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeB, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexB, 3);
	
	const auto world_manifold = GetWorldManifold(manifold, xfm0, GetRadius(shape0), xfm1, GetRadius(shape1));
	EXPECT_EQ(world_manifold.GetPointCount(), Manifold::size_type(2));
	
	EXPECT_FLOAT_EQ(world_manifold.GetNormal().x, float_t(1));
	EXPECT_FLOAT_EQ(world_manifold.GetNormal().y, float_t(0));
	
	ASSERT_GT(world_manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).x, float_t(-0.5));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).y, float_t(+1.5));

	ASSERT_GT(world_manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(1).x, float_t(-0.5));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(1).y, float_t(-1.5));
}

TEST(CollideShapes, HorizontalOverlappingRects2)
{
	// wide rectangle
	const auto shape0 = PolygonShape(3, 1.5);
	ASSERT_EQ(shape0.GetVertex(0), Vec2(float_t(+3.0), float_t(-1.5))); // bottom right
	ASSERT_EQ(shape0.GetVertex(1), Vec2(float_t(+3.0), float_t(+1.5))); // top right
	ASSERT_EQ(shape0.GetVertex(2), Vec2(float_t(-3.0), float_t(+1.5))); // top left
	ASSERT_EQ(shape0.GetVertex(3), Vec2(float_t(-3.0), float_t(-1.5))); // bottom left
	
	// square
	const auto shape1 = PolygonShape(2, 2);
	ASSERT_EQ(shape1.GetVertex(0), Vec2(+2,-2)); // bottom right
	ASSERT_EQ(shape1.GetVertex(1), Vec2(+2,+2)); // top right
	ASSERT_EQ(shape1.GetVertex(2), Vec2(-2,+2)); // top left
	ASSERT_EQ(shape1.GetVertex(3), Vec2(-2,-2)); // bottom left
	
	const auto xfm0 = Transformation(Vec2{-2, 0}, Rot{0}); // left
	const auto xfm1 = Transformation(Vec2{+2, 0}, Rot{0}); // right

	// put wide rectangle on left, square on right
	const auto manifold = CollideShapes(shape0, xfm0, shape1, xfm1);
	
	EXPECT_EQ(manifold.GetType(), Manifold::e_faceA);
	
	EXPECT_EQ(manifold.GetLocalPoint(), Vec2(float_t(+3), float_t(0)));
	EXPECT_EQ(manifold.GetLocalNormal(), Vec2(float_t(+1), float_t(0)));
	
	EXPECT_EQ(manifold.GetPointCount(), Manifold::size_type(2));
	
	const auto total_radius = GetRadius(shape0) + GetRadius(shape1);

	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).localPoint.x, float_t(-2.0)); // left
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).localPoint.y, float_t(-1.5) - total_radius); // top
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).normalImpulse, float_t(0));
	EXPECT_FLOAT_EQ(manifold.GetPoint(0).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexA, 0);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.typeB, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(0).contactFeature.indexB, 2);
	
	ASSERT_GT(manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_FLOAT_EQ(manifold.GetPoint(1).localPoint.x, float_t(-2.0)); // left
	EXPECT_FLOAT_EQ(manifold.GetPoint(1).localPoint.y, float_t(+1.5) + total_radius); // bottom
	EXPECT_FLOAT_EQ(manifold.GetPoint(1).normalImpulse, float_t(0));
	EXPECT_FLOAT_EQ(manifold.GetPoint(1).tangentImpulse, float_t(0));
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeA, ContactFeature::e_vertex);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexA, 1);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.typeB, ContactFeature::e_face);
	EXPECT_EQ(manifold.GetPoint(1).contactFeature.indexB, 2);
	
	const auto world_manifold = GetWorldManifold(manifold, xfm0, GetRadius(shape0), xfm1, GetRadius(shape1));
	EXPECT_EQ(world_manifold.GetPointCount(), Manifold::size_type(2));
	
	EXPECT_FLOAT_EQ(world_manifold.GetNormal().x, float_t(1));
	EXPECT_FLOAT_EQ(world_manifold.GetNormal().y, float_t(0));
	
	ASSERT_GT(world_manifold.GetPointCount(), Manifold::size_type(0));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).x, float_t(+0.5));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(0).y, float_t(-1.5) - total_radius);
	
	ASSERT_GT(world_manifold.GetPointCount(), Manifold::size_type(1));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(1).x, float_t(+0.5));
	EXPECT_FLOAT_EQ(world_manifold.GetPoint(1).y, float_t(+1.5) + total_radius);
}