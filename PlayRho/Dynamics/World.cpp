/*
 * Original work Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#include <PlayRho/Dynamics/World.hpp>

#include <PlayRho/Dynamics/WorldImplBody.hpp>
#include <PlayRho/Dynamics/WorldImplContact.hpp>
#include <PlayRho/Dynamics/WorldImplFixture.hpp>
#include <PlayRho/Dynamics/WorldImplJoint.hpp>
#include <PlayRho/Dynamics/WorldImplMisc.hpp>

#include <PlayRho/Dynamics/BodyConf.hpp>
#include <PlayRho/Dynamics/StepConf.hpp>

namespace playrho {
namespace d2 {

static_assert(std::is_default_constructible<World>::value, "World must be default constructible!");
static_assert(std::is_copy_constructible<World>::value, "World must be copy constructible!");
static_assert(std::is_copy_assignable<World>::value, "World must be copy assignable!");
static_assert(std::is_move_constructible<World>::value, "World must be move constructible!");
static_assert(std::is_move_assignable<World>::value, "World must be move assignable!");
static_assert(std::is_nothrow_destructible<World>::value, "World must be nothrow destructible!");

// Special member functions are off in their own .cpp file to avoid their
// necessary includes being in this file!!

void World::SetFixtureDestructionListener(const FixtureListener& listener) noexcept
{
    ::playrho::d2::SetFixtureDestructionListener(*m_impl, listener);
}

void World::SetJointDestructionListener(const JointListener& listener) noexcept
{
    ::playrho::d2::SetJointDestructionListener(*m_impl, listener);
}

void World::SetBeginContactListener(ContactListener listener) noexcept
{
    ::playrho::d2::SetBeginContactListener(*m_impl, listener);
}

void World::SetEndContactListener(ContactListener listener) noexcept
{
    ::playrho::d2::SetEndContactListener(*m_impl, listener);
}

void World::SetPreSolveContactListener(ManifoldContactListener listener) noexcept
{
    ::playrho::d2::SetPreSolveContactListener(*m_impl, listener);
}

void World::SetPostSolveContactListener(ImpulsesContactListener listener) noexcept
{
    ::playrho::d2::SetPostSolveContactListener(*m_impl, listener);
}

void World::Clear() noexcept
{
    ::playrho::d2::Clear(*m_impl);
}

StepStats World::Step(const StepConf& conf)
{
    return ::playrho::d2::Step(*m_impl, conf);
}

bool World::IsStepComplete() const noexcept
{
    return ::playrho::d2::IsStepComplete(*m_impl);
}

bool World::GetSubStepping() const noexcept
{
    return ::playrho::d2::GetSubStepping(*m_impl);
}

void World::SetSubStepping(bool flag) noexcept
{
    ::playrho::d2::SetSubStepping(*m_impl, flag);
}

const DynamicTree& World::GetTree() const noexcept
{
    return ::playrho::d2::GetTree(*m_impl);
}

bool World::IsLocked() const noexcept
{
    return m_impl && ::playrho::d2::IsLocked(*m_impl);
}

void World::ShiftOrigin(Length2 newOrigin)
{
    ::playrho::d2::ShiftOrigin(*m_impl, newOrigin);
}

Length World::GetMinVertexRadius() const noexcept
{
    return ::playrho::d2::GetMinVertexRadius(*m_impl);
}

Length World::GetMaxVertexRadius() const noexcept
{
    return ::playrho::d2::GetMaxVertexRadius(*m_impl);
}

Frequency World::GetInvDeltaTime() const noexcept
{
    return ::playrho::d2::GetInvDeltaTime(*m_impl);
}

FixtureCounter World::GetShapeCount() const noexcept
{
    return ::playrho::d2::GetShapeCount(*m_impl);
}

BodyCounter World::GetBodyRange() const noexcept
{
    return ::playrho::d2::GetBodyRange(*m_impl);
}

SizedRange<World::Bodies::const_iterator> World::GetBodies() const noexcept
{
    return ::playrho::d2::GetBodies(*m_impl);
}

SizedRange<World::Bodies::const_iterator> World::GetBodiesForProxies() const noexcept
{
    return ::playrho::d2::GetBodiesForProxies(*m_impl);
}

BodyID World::CreateBody(const BodyConf& def)
{
    return ::playrho::d2::CreateBody(*m_impl, def);
}

const Body& World::GetBody(BodyID id) const
{
    return ::playrho::d2::GetBody(*m_impl, id);
}

void World::SetBody(BodyID id, const Body& value)
{
    ::playrho::d2::SetBody(*m_impl, id, value);
}

void World::Destroy(BodyID id)
{
    ::playrho::d2::Destroy(*m_impl, id);
}

SizedRange<World::Fixtures::const_iterator> World::GetFixtures(BodyID id) const
{
    return ::playrho::d2::GetFixtures(*m_impl, id);
}

SizedRange<World::BodyJoints::const_iterator> World::GetJoints(BodyID id) const
{
    return ::playrho::d2::GetJoints(*m_impl, id);
}

SizedRange<World::Contacts::const_iterator> World::GetContacts(BodyID id) const
{
    return ::playrho::d2::GetContacts(*m_impl, id);
}

FixtureID World::CreateFixture(const FixtureConf& def)
{
    return ::playrho::d2::CreateFixture(*m_impl, def);
}

const FixtureConf& World::GetFixture(FixtureID id) const
{
    return ::playrho::d2::GetFixture(*m_impl, id);
}

void World::SetFixture(FixtureID id, const FixtureConf& value)
{
    ::playrho::d2::SetFixture(*m_impl, id, value);
}

bool World::Destroy(FixtureID id)
{
    return ::playrho::d2::Destroy(*m_impl, id);
}

SizedRange<World::Joints::const_iterator> World::GetJoints() const noexcept
{
    return ::playrho::d2::GetJoints(*m_impl);
}

JointID World::CreateJoint(const Joint& def)
{
    return ::playrho::d2::CreateJoint(*m_impl, def);
}

const Joint& World::GetJoint(JointID id) const
{
    return ::playrho::d2::GetJoint(*m_impl, id);
}

void World::SetJoint(JointID id, const Joint& def)
{
    ::playrho::d2::SetJoint(*m_impl, id, def);
}

void World::Destroy(JointID id)
{
    ::playrho::d2::Destroy(*m_impl, id);
}

SizedRange<World::Contacts::const_iterator> World::GetContacts() const noexcept
{
    return ::playrho::d2::GetContacts(*m_impl);
}

bool World::IsAwake(ContactID id) const
{
    return ::playrho::d2::IsAwake(*m_impl, id);
}

LinearVelocity World::GetTangentSpeed(ContactID id) const
{
    return ::playrho::d2::GetTangentSpeed(*m_impl, id);
}

void World::SetTangentSpeed(ContactID id, LinearVelocity value)
{
    ::playrho::d2::SetTangentSpeed(*m_impl, id, value);
}

bool World::IsTouching(ContactID id) const
{
    return ::playrho::d2::IsTouching(*m_impl, id);
}

bool World::NeedsFiltering(ContactID id) const
{
    return ::playrho::d2::NeedsFiltering(*m_impl, id);
}

bool World::NeedsUpdating(ContactID id) const
{
    return ::playrho::d2::NeedsUpdating(*m_impl, id);
}

bool World::HasValidToi(ContactID id) const
{
    return ::playrho::d2::HasValidToi(*m_impl, id);
}

Real World::GetToi(ContactID id) const
{
    return ::playrho::d2::GetToi(*m_impl, id);
}

FixtureID World::GetFixtureA(ContactID id) const
{
    return ::playrho::d2::GetFixtureA(*m_impl, id);
}

FixtureID World::GetFixtureB(ContactID id) const
{
    return ::playrho::d2::GetFixtureB(*m_impl, id);
}

BodyID World::GetBodyA(ContactID id) const
{
    return ::playrho::d2::GetBodyA(*m_impl, id);
}

BodyID World::GetBodyB(ContactID id) const
{
    return ::playrho::d2::GetBodyB(*m_impl, id);
}

ChildCounter World::GetChildIndexA(ContactID id) const
{
    return ::playrho::d2::GetChildIndexA(*m_impl, id);
}

ChildCounter World::GetChildIndexB(ContactID id) const
{
    return ::playrho::d2::GetChildIndexB(*m_impl, id);
}

TimestepIters World::GetToiCount(ContactID id) const
{
    return ::playrho::d2::GetToiCount(*m_impl, id);
}

Real World::GetDefaultFriction(ContactID id) const
{
    return ::playrho::d2::GetDefaultFriction(*m_impl, id);
}

Real World::GetDefaultRestitution(ContactID id) const
{
    return ::playrho::d2::GetDefaultRestitution(*m_impl, id);
}

Real World::GetFriction(ContactID id) const
{
    return ::playrho::d2::GetFriction(*m_impl, id);
}

Real World::GetRestitution(ContactID id) const
{
    return ::playrho::d2::GetRestitution(*m_impl, id);
}

void World::SetFriction(ContactID id, Real value)
{
    ::playrho::d2::SetFriction(*m_impl, id, value);
}

void World::SetRestitution(ContactID id, Real value)
{
    ::playrho::d2::SetRestitution(*m_impl, id, value);
}

const Manifold& World::GetManifold(ContactID id) const
{
    return ::playrho::d2::GetManifold(*m_impl, id);
}

bool World::IsEnabled(ContactID id) const
{
    return ::playrho::d2::IsEnabled(*m_impl, id);
}

void World::SetEnabled(ContactID id)
{
    ::playrho::d2::SetEnabled(*m_impl, id);
}

void World::UnsetEnabled(ContactID id)
{
    ::playrho::d2::UnsetEnabled(*m_impl, id);
}

} // namespace d2
} // namespace playrho
