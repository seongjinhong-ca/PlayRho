/*
* Original work Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#include <Box2D/Collision/Shapes/EdgeShape.h>
#include <new>

using namespace box2d;

void EdgeShape::Set(const Vec2& v1, const Vec2& v2)
{
	m_vertex1 = v1;
	m_vertex2 = v2;
	m_vertex0 = GetInvalid<Vec2>();
	m_vertex3 = GetInvalid<Vec2>();
}

child_count_t box2d::GetChildCount(const EdgeShape& shape)
{
	return 1;
}

bool box2d::TestPoint(const EdgeShape& shape, const Transformation& xf, const Vec2& p)
{
	BOX2D_NOT_USED(xf);
	BOX2D_NOT_USED(p);
	return false;
}

AABB box2d::ComputeAABB(const EdgeShape& shape, const Transformation& xf, child_count_t childIndex)
{
	BOX2D_NOT_USED(childIndex);

	const auto v1 = Transform(shape.GetVertex1(), xf);
	const auto v2 = Transform(shape.GetVertex2(), xf);

	const auto lower = Min(v1, v2);
	const auto upper = Max(v1, v2);

	const auto r = Vec2{GetVertexRadius(shape), GetVertexRadius(shape)};
	return AABB{lower - r, upper + r};
}

MassData box2d::ComputeMass(const EdgeShape& shape, float_t density)
{
	BOX2D_NOT_USED(density);

	return MassData{float_t{0}, (shape.GetVertex1() + shape.GetVertex2()) / float_t(2), float_t{0}};
}
