/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
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

#ifndef VERTICAL_STACK_H
#define VERTICAL_STACK_H

namespace box2d {

extern bool g_blockSolve;

class VerticalStack : public Test
{
public:

	enum
	{
		e_columnCount = 1,
		e_rowCount = 15
		//e_columnCount = 1,
		//e_rowCount = 1
	};

	VerticalStack()
	{
		{
			BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2EdgeShape shape;
			shape.Set(b2Vec2(-40.0f, 0.0f), b2Vec2(40.0f, 0.0f));
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(b2Vec2(20.0f, 0.0f), b2Vec2(20.0f, 20.0f));
			ground->CreateFixture(&shape, 0.0f);
		}

		float_t xs[5] = {0.0f, -10.0f, -5.0f, 5.0f, 10.0f};

		for (int32 j = 0; j < e_columnCount; ++j)
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.5f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 1.0f;
			fd.friction = 0.3f;

			for (int i = 0; i < e_rowCount; ++i)
			{
				BodyDef bd;
				bd.type = DynamicBody;

				int32 n = j * e_rowCount + i;
				assert(n < e_rowCount * e_columnCount);
				m_indices[n] = n;
				bd.userData = m_indices + n;

				float_t x = 0.0f;
				//float_t x = RandomFloat(-0.02f, 0.02f);
				//float_t x = i % 2 == 0 ? -0.01f : 0.01f;
				bd.position = b2Vec2(xs[j] + x, 0.55f + 1.1f * i);
				b2Body* body = m_world->CreateBody(&bd);

				m_bodies[n] = body;

				body->CreateFixture(&fd);
			}
		}

		m_bullet = nullptr;
	}

	void Keyboard(int key)
	{
		switch (key)
		{
		case GLFW_KEY_COMMA:
			if (m_bullet != nullptr)
			{
				m_world->DestroyBody(m_bullet);
				m_bullet = nullptr;
			}

			{
				b2CircleShape shape;
				shape.SetRadius(float_t(0.25));

				b2FixtureDef fd;
				fd.shape = &shape;
				fd.density = 20.0f;
				fd.restitution = 0.05f;

				BodyDef bd;
				bd.type = DynamicBody;
				bd.bullet = true;
				bd.position = b2Vec2(-31.0f, 5.0f);

				m_bullet = m_world->CreateBody(&bd);
				m_bullet->CreateFixture(&fd);

				m_bullet->SetLinearVelocity(b2Vec2(400.0f, 0.0f));
			}
			break;
                
        case GLFW_KEY_B:
            g_blockSolve = !g_blockSolve;
            break;
		}
	}

	void Step(Settings* settings)
	{
		Test::Step(settings);
		g_debugDraw.DrawString(5, m_textLine, "Press: (,) to launch a bullet.");
		m_textLine += DRAW_STRING_NEW_LINE;
		g_debugDraw.DrawString(5, m_textLine, "Blocksolve = %d", g_blockSolve);
		//if (m_stepCount == 300)
		//{
		//	if (m_bullet != nullptr)
		//	{
		//		m_world->DestroyBody(m_bullet);
		//		m_bullet = nullptr;
		//	}

		//	{
		//		b2CircleShape shape;
		//		shape.m_radius = 0.25f;

		//		b2FixtureDef fd;
		//		fd.shape = &shape;
		//		fd.density = 20.0f;
		//		fd.restitution = 0.05f;

		//		BodyDef bd;
		//		bd.type = DynamicBody;
		//		bd.bullet = true;
		//		bd.position = b2Vec2(-31.0f, 5.0f);

		//		m_bullet = m_world->CreateBody(&bd);
		//		m_bullet->CreateFixture(&fd);

		//		m_bullet->SetLinearVelocity(b2Vec2(400.0f, 0.0f));
		//	}
		//}
	}

	static Test* Create()
	{
		return new VerticalStack;
	}

	b2Body* m_bullet;
	b2Body* m_bodies[e_rowCount * e_columnCount];
	int32 m_indices[e_rowCount * e_columnCount];
};
	
} // namespace box2d

#endif
