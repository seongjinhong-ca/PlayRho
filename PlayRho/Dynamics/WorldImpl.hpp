/*
 * Original work Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2023 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_DYNAMICS_WORLDIMPL_HPP
#define PLAYRHO_DYNAMICS_WORLDIMPL_HPP

/// @file
/// Declarations of the WorldImpl class.

#include <PlayRho/Common/Interval.hpp>
#include <PlayRho/Common/Math.hpp>
#include <PlayRho/Common/PoolMemoryResource.hpp>
#include <PlayRho/Common/Positive.hpp>
#include <PlayRho/Common/ObjectPool.hpp>

#include <PlayRho/Collision/DynamicTree.hpp>
#include <PlayRho/Collision/MassData.hpp>

#include <PlayRho/Dynamics/BodyID.hpp>
#include <PlayRho/Dynamics/Filter.hpp>
#include <PlayRho/Dynamics/Island.hpp>
#include <PlayRho/Dynamics/StepStats.hpp>
#include <PlayRho/Dynamics/Contacts/BodyConstraint.hpp>
#include <PlayRho/Dynamics/Contacts/ContactKey.hpp>
#include <PlayRho/Dynamics/Contacts/KeyedContactID.hpp>
#include <PlayRho/Dynamics/WorldConf.hpp>
#include <PlayRho/Dynamics/Joints/JointID.hpp>
#include <PlayRho/Dynamics/IslandStats.hpp>
#include <PlayRho/Collision/Shapes/ShapeID.hpp>

#include <iterator>
#include <vector>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <functional>
#include <type_traits> // for std::is_default_constructible_v, etc.

namespace playrho {

struct StepConf;
enum class BodyType;
class Contact;

namespace d2 {

class Body;
class Joint;
class Shape;
class Manifold;
class ContactImpulsesList;

/// @brief Definition of a "world" implementation.
/// @see World.
class WorldImpl {
public:
    /// @brief Bodies container type.
    using Bodies = std::vector<BodyID>;

    /// @brief Contacts container type.
    using Contacts = std::vector<KeyedContactID>;

    /// @brief Joints container type.
    /// @note Cannot be container of Joint instances since joints are polymorphic types.
    using Joints = std::vector<JointID>;

    /// @brief Container type for Body associated contact information.
    using BodyContacts = std::vector<std::tuple<ContactKey, ContactID>>;

    /// @brief Body joints container type.
    using BodyJoints = std::vector<std::pair<BodyID, JointID>>;

    /// @brief Proxy ID type alias.
    using ProxyId = DynamicTree::Size;

    /// @brief Proxy container type alias.
    using Proxies = std::vector<ProxyId>;

    /// @brief Shape listener.
    using ShapeListener = std::function<void(ShapeID)>;

    /// @brief Body-shape listener.
    using AssociationListener = std::function<void(std::pair<BodyID, ShapeID>)>;

    /// @brief Joint listener.
    using JointListener = std::function<void(JointID)>;

    /// @brief Contact listener.
    using ContactListener = std::function<void(ContactID)>;

    /// @brief Manifold contact listener.
    using ManifoldContactListener = std::function<void(ContactID, const Manifold&)>;

    /// @brief Impulses contact listener.
    using ImpulsesContactListener = std::function<void(ContactID, const ContactImpulsesList&, unsigned)>;

    /// @brief Broad phase generated data for identifying potentially new contacts.
    /// @details Stores the contact-key followed by the key's min contactable then max contactable data.
    using ProxyKey = std::tuple<ContactKey, Contactable, Contactable>;

    struct ContactUpdateConf;

    /// @name Special Member Functions
    /// Special member functions that are explicitly defined.
    /// @{

    /// @brief Constructs a world implementation for a world.
    /// @param conf A customized world configuration or its default value.
    /// @note A lot more configurability can be had via the <code>StepConf</code>
    ///   data that's given to the world's <code>Step</code> method.
    /// @throws InvalidArgument if the given max vertex radius is less than the min.
    /// @see Step.
    explicit WorldImpl(const WorldConf& conf = WorldConf{});

    /// @brief Copy constructor.
    WorldImpl(const WorldImpl& other);

    /// @brief Destructor.
    /// @details All physics entities are destroyed and all memory is released.
    /// @note This will call the <code>Clear()</code> function.
    /// @see Clear.
    ~WorldImpl() noexcept;

    // Delete compiler defined implementations of move construction/assignment and copy assignment...
    WorldImpl(WorldImpl&& other) = delete;
    WorldImpl& operator=(const WorldImpl& other) = delete;
    WorldImpl& operator=(WorldImpl&& other) = delete;

    /// @}

    /// @name Listener Member Functions
    /// @{

    /// @brief Registers a destruction listener for shapes.
    void SetShapeDestructionListener(ShapeListener listener) noexcept;

    /// @brief Registers a detach listener for shapes detaching from bodies.
    void SetDetachListener(AssociationListener listener) noexcept;

    /// @brief Register a destruction listener for joints.
    void SetJointDestructionListener(JointListener listener) noexcept;

    /// @brief Register a begin contact event listener.
    void SetBeginContactListener(ContactListener listener) noexcept;

    /// @brief Register an end contact event listener.
    void SetEndContactListener(ContactListener listener) noexcept;

    /// @brief Register a pre-solve contact event listener.
    void SetPreSolveContactListener(ManifoldContactListener listener) noexcept;

    /// @brief Register a post-solve contact event listener.
    void SetPostSolveContactListener(ImpulsesContactListener listener) noexcept;

    /// @}

    /// @name Miscellaneous Member Functions
    /// @{

    /// @brief Clears this world.
    /// @post The contents of this world have all been destroyed and this world's internal
    ///   state reset as though it had just been constructed.
    void Clear() noexcept;

    /// @brief Steps the world simulation according to the given configuration.
    ///
    /// @details
    /// Performs position and velocity updating, sleeping of non-moving bodies, updating
    /// of the contacts, and notifying the contact listener of begin-contact, end-contact,
    /// pre-solve, and post-solve events.
    ///
    /// @warning Behavior is undefined if given a negative step time delta.
    /// @warning Varying the step time delta may lead to non-physical behaviors.
    ///
    /// @note Calling this with a zero step time delta results only in fixtures and bodies
    ///   registered for proxy handling being processed. No physics is performed.
    /// @note If the given velocity and position iterations are zero, this method doesn't
    ///   do velocity or position resolutions respectively of the contacting bodies.
    /// @note While body velocities are updated accordingly (per the sum of forces acting on them),
    ///   body positions (barring any collisions) are updated as if they had moved the entire time
    ///   step at those resulting velocities. In other words, a body initially at position 0
    ///   (<code>p0</code>) going velocity 0 (<code>v0</code>) fast with a sum acceleration of
    ///   <code>a</code>, after time <code>t</code> and barring any collisions, will have a new
    ///   velocity (<code>v1</code>) of <code>v0 + (a * t)</code> and a new position
    ///   (<code>p1</code>) of <code>p0 + v1 * t</code>.
    ///
    /// @post Static bodies are unmoved.
    /// @post Kinetic bodies are moved based on their previous velocities.
    /// @post Dynamic bodies are moved based on their previous velocities, gravity, applied
    ///   forces, applied impulses, masses, damping, and the restitution and friction values
    ///   of their fixtures when they experience collisions.
    /// @post The bodies for proxies queue will be empty.
    /// @post The fixtures for proxies queue will be empty.
    ///
    /// @param conf Configuration for the simulation step.
    ///
    /// @return Statistics for the step.
    ///
    /// @throws WrongState if this function is called while the world is locked.
    ///
    /// @see GetBodiesForProxies, GetFixturesForProxies.
    ///
    StepStats Step(const StepConf& conf);

    /// @brief Whether or not "step" is complete.
    /// @details The "step" is completed when there are no more TOI events for the current time step.
    /// @return <code>true</code> unless sub-stepping is enabled and the step method returned
    ///   without finishing all of its sub-steps.
    /// @see GetSubStepping, SetSubStepping.
    bool IsStepComplete() const noexcept;

    /// @brief Gets whether or not sub-stepping is enabled.
    /// @see SetSubStepping, IsStepComplete.
    bool GetSubStepping() const noexcept;

    /// @brief Enables/disables single stepped continuous physics.
    /// @note This is not normally used. Enabling sub-stepping is meant for testing.
    /// @post The <code>GetSubStepping()</code> method will return the value this method was
    ///   called with.
    /// @see IsStepComplete, GetSubStepping.
    void SetSubStepping(bool flag) noexcept;

    /// @brief Gets access to the broad-phase dynamic tree information.
    const DynamicTree& GetTree() const noexcept;

    /// @brief Is the world locked (in the middle of a time step).
    bool IsLocked() const noexcept;

    /// @brief Shifts the world origin.
    /// @note Useful for large worlds.
    /// @note The body shift formula is: <code>position -= newOrigin</code>.
    /// @post The "origin" of this world's bodies, joints, and the board-phase dynamic tree
    ///   have been translated per the shift amount and direction.
    /// @param newOrigin the new origin with respect to the old origin
    /// @throws WrongState if this function is called while the world is locked.
    void ShiftOrigin(const Length2& newOrigin);

    /// @brief Gets the minimum vertex radius that shapes in this world can be.
    Length GetMinVertexRadius() const noexcept;

    /// @brief Gets the maximum vertex radius that shapes in this world can be.
    Length GetMaxVertexRadius() const noexcept;

    /// @brief Gets the inverse delta time.
    /// @details Gets the inverse delta time that was set on construction or assignment, and
    ///   updated on every call to the <code>Step()</code> method having a non-zero delta-time.
    /// @see Step.
    Frequency GetInvDeltaTime() const noexcept;

    /// @brief Gets the dynamic tree leaves queued for finding new contacts.
    const Proxies& GetProxies() const noexcept;

    /// @brief Gets the fixtures-for-proxies for this world.
    /// @details Provides insight on what fixtures have been queued for proxy processing
    ///   during the next call to the world step method.
    /// @see Step.
    const std::vector<std::pair<BodyID, ShapeID>>& GetFixturesForProxies() const noexcept;

    /// @}

    /// @name Body Member Functions
    /// Member functions relating to bodies.
    /// @{

    /// @brief Gets the extent of the currently valid body range.
    /// @note This is one higher than the maxium <code>BodyID</code> that is in range
    ///   for body related functions.
    BodyCounter GetBodyRange() const noexcept;

    /// @brief Gets the world body range for this constant world.
    /// @details Gets a range enumerating the bodies currently existing within this world.
    ///   These are the bodies that had been created from previous calls to the
    ///   <code>CreateBody(const Body&)</code> method that haven't yet been destroyed.
    /// @return Body range that can be iterated over using its begin and end methods
    ///   or using ranged-based for-loops.
    /// @see CreateBody(const Body&).
    const Bodies& GetBodies() const noexcept;

    /// @brief Gets the bodies-for-proxies range for this world.
    /// @details Provides insight on what bodies have been queued for proxy processing
    ///   during the next call to the world step method.
    /// @see Step.
    const Bodies& GetBodiesForProxies() const noexcept;

    /// @brief Creates a rigid body that's a copy of the given one.
    /// @warning This function should not be used while the world is locked &mdash; as it is
    ///   during callbacks. If it is, it will throw an exception or abort your program.
    /// @note No references to the configuration are retained. Its value is copied.
    /// @post The created body will be present in the range returned from the
    ///   <code>GetBodies()</code> method.
    /// @param body A customized body or its default value.
    /// @return Identifier of the newly created body which can later be destroyed by calling
    ///   the <code>Destroy(BodyID)</code> method.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws LengthError if this operation would create more than <code>MaxBodies</code>.
    /// @throws std::out_of_range if the given body references any invalid shape identifiers.
    /// @see Destroy(BodyID), GetBodies.
    /// @see PhysicalEntities.
    BodyID CreateBody(Body body);

    /// @brief Gets the identified body.
    /// @throws std::out_of_range if given an invalid id.
    /// @see SetBody, GetBodyRange.
    const Body& GetBody(BodyID id) const;

    /// @brief Sets the identified body.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws std::out_of_range if given an invalid id of if the given body references any
    ///   invalid shape identifiers.
    /// @throws InvalidArgument if the specified ID was destroyed.
    /// @see GetBody, GetBodyRange.
    void SetBody(BodyID id, Body value);

    /// @brief Destroys the identified body.
    /// @details Destroys a given body that had previously been created by a call to this
    ///   world's <code>CreateBody(const Body&)</code> method.
    /// @warning This automatically deletes all associated shapes and joints.
    /// @warning This function is locked during callbacks.
    /// @warning Behavior is undefined if given a null body.
    /// @warning Behavior is undefined if the passed body was not created by this world.
    /// @note This function is locked during callbacks.
    /// @post The destroyed body will no longer be present in the range returned from the
    ///   <code>GetBodies()</code> method.
    /// @post None of the body's fixtures will be present in the fixtures-for-proxies
    ///   collection.
    /// @param id Body to destroy that had been created by this world.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws std::out_of_range If given an invalid body identifier.
    /// @see CreateBody(const Body&), GetBodies, GetFixturesForProxies.
    /// @see PhysicalEntities.
    void Destroy(BodyID id);

    /// @brief Gets whether the given identifier is to a body that's been destroyed.
    /// @note Complexity is at most O(n) where n is the number of elements free.
    bool IsDestroyed(BodyID id) const noexcept;

    /// @brief Gets the proxies for the identified body.
    /// @throws std::out_of_range If given an invalid identifier.
    const Proxies& GetProxies(BodyID id) const;

    /// @brief Gets the contacts associated with the identified body.
    /// @throws std::out_of_range if given an invalid id.
    const BodyContacts& GetContacts(BodyID id) const;

    /// @throws std::out_of_range if given an invalid id.
    const BodyJoints& GetJoints(BodyID id) const;

    /// @}

    /// @name Joint Member Functions
    /// Member functions relating to joints.
    /// @{

    /// @brief Gets the extent of the currently valid joint range.
    /// @note This is one higher than the maxium <code>JointID</code> that is in range
    ///   for joint related functions.
    JointCounter GetJointRange() const noexcept;

    /// @brief Gets the world joint range.
    /// @details Gets a range enumerating the joints currently existing within this world.
    ///   These are the joints that had been created from previous calls to the
    ///   <code>CreateJoint(const Joint&)</code> method that haven't yet been destroyed.
    /// @return World joints sized-range.
    /// @see CreateJoint(const Joint&).
    const Joints& GetJoints() const noexcept;

    /// @brief Creates a joint to constrain one or more bodies.
    /// @warning This function is locked during callbacks.
    /// @note No references to the configuration are retained. Its value is copied.
    /// @post The created joint will be present in the range returned from the
    ///   <code>GetJoints()</code> method.
    /// @return Identifier for the newly created joint which can later be destroyed by calling
    ///   the <code>Destroy(JointID)</code> method.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws LengthError if this operation would create more than <code>MaxJoints</code>.
    /// @throws InvalidArgument if the given definition is not allowed.
    /// @throws std::out_of_range if the given joint references any invalid body id.
    /// @see PhysicalEntities.
    /// @see Destroy(JointID), GetJoints.
    JointID CreateJoint(Joint def);

    /// @brief Gets the identified joint.
    /// @throws std::out_of_range if given an invalid ID.
    const Joint& GetJoint(JointID id) const;

    /// @brief Sets the identified joint.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws std::out_of_range if given an invalid ID or the given joint references any
    ///    invalid body ID.
    /// @throws InvalidArgument if the specified ID was destroyed.
    /// @see CreateJoint(Joint def), Destroy(JointID joint).
    void SetJoint(JointID id, Joint def);

    /// @brief Destroys a joint.
    /// @details Destroys a given joint that had previously been created by a call to this
    ///   world's <code>CreateJoint(const Joint&)</code> method.
    /// @warning This function is locked during callbacks.
    /// @warning Behavior is undefined if the passed joint was not created by this world.
    /// @note This may cause the connected bodies to begin colliding.
    /// @post The destroyed joint will no longer be present in the range returned from the
    ///   <code>GetJoints()</code> method.
    /// @param id Identifier of joint to destroy that had been created by this world.
    /// @throws WrongState if this function is called while the world is locked.
    /// @see CreateJoint(const Joint&), GetJoints.
    /// @see PhysicalEntities.
    void Destroy(JointID id);

    /// @brief Gets whether the given identifier is to a joint that's been destroyed.
    /// @note Complexity is at most O(n) where n is the number of elements free.
    bool IsDestroyed(JointID id) const noexcept;

    /// @}

    /// @name Shape Member Functions
    /// Member functions relating to shapes.
    /// @{

    /// @brief Gets the extent of the currently valid shape range.
    /// @note This is one higher than the maxium <code>ShapeID</code> that is in range
    ///   for shape related functions.
    ShapeCounter GetShapeRange() const noexcept;

    /// @brief Creates an identifiable copy of the given shape within this world.
    /// @throws InvalidArgument if called for a shape with a vertex radius that's either:
    ///    less than the minimum vertex radius, or greater than the maximum vertex radius.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws LengthError if this operation would create more than <code>MaxShapes</code>.
    /// @see Destroy(ShapeID), GetShape, SetShape.
    ShapeID CreateShape(Shape def);

    /// @brief Gets the identified shape.
    /// @throws std::out_of_range If given an invalid shape identifier.
    /// @see CreateShape.
    const Shape& GetShape(ShapeID id) const;

    /// @brief Sets the value of the identified shape.
    /// @warning This function is locked during callbacks.
    /// @note This function does not reset the mass data of any effected bodies.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws std::out_of_range If given an invalid identifier.
    /// @throws InvalidArgument if the specified ID was destroyed.
    /// @see CreateShape, Destroy(ShapeID id).
    void SetShape(ShapeID id, Shape def);

    /// @brief Destroys the identified shape removing any body associations with it first.
    /// @warning This function is locked during callbacks.
    /// @note This function does not reset the mass data of any effected bodies.
    /// @throws WrongState if this function is called while the world is locked.
    /// @throws std::out_of_range If given an invalid shape identifier.
    /// @see CreateShape, Detach.
    void Destroy(ShapeID id);

    /// @}

    /// @name Contact Member Functions
    /// Member functions relating to contacts.
    /// @{

    /// @brief Gets the extent of the currently valid contact range.
    /// @note This is one higher than the maxium <code>ContactID</code> that is in range
    ///   for contact related functions.
    ContactCounter GetContactRange() const noexcept;

    /// @brief Gets the world contact range.
    /// @warning contacts are created and destroyed in the middle of a time step.
    /// Use <code>ContactListener</code> to avoid missing contacts.
    /// @return World contacts sized-range.
    std::vector<KeyedContactID> GetContacts() const;

    /// @brief Gets the identified contact.
    /// @throws std::out_of_range If given an invalid contact identifier.
    /// @see SetContact.
    const Contact& GetContact(ContactID id) const;

    /// @brief Sets the identified contact's state.
    /// @note This will throw an exception to preserve invariants.
    /// @invariant A contact may only be impenetrable if one or both bodies are.
    /// @invariant A contact may only be active if one or both bodies are awake.
    /// @invariant A contact may only be a sensor or one or both shapes are.
    /// @throws InvalidArgument if a change would violate an invariant or if the specified ID
    ///   was destroyed.
    /// @throws std::out_of_range If given an invalid contact identifier or an invalid identifier
    ///   in the new contact value.
    /// @see GetContact.
    void SetContact(ContactID id, Contact value);

    /// @brief Gets the identified manifold.
    /// @throws std::out_of_range If given an invalid contact identifier.
    const Manifold& GetManifold(ContactID id) const;

    /// @brief Gets whether the given identifier is to a contact that's been destroyed.
    /// @note Complexity is at most O(n) where n is the number of elements free.
    bool IsDestroyed(ContactID id) const noexcept;

    /// @}

private:
    /// @brief Flags type data type.
    using FlagsType = std::uint32_t;

    /// @brief Flag enumeration.
    enum Flag: FlagsType
    {
        /// Locked.
        e_locked = 0x0002,

        /// Sub-stepping.
        e_substepping = 0x0020,

        /// Step complete. @details Used for sub-stepping. @see e_substepping.
        e_stepComplete = 0x0040,

        /// Needs contact filtering.
        e_needsContactFiltering = 0x0080,
    };

    /// Bodies, contacts, and joints that are already in an <code>Island</code> by their ID.
    /// @see Step.
    struct Islanded
    {
        using vector = std::vector<bool>;
        vector bodies;
        vector contacts;
        vector joints;
    };

    /// @brief Solves the step.
    /// @details Finds islands, integrates and solves constraints, solves position constraints.
    /// @note This may miss collisions involving fast moving bodies and allow them to tunnel
    ///   through each other.
    /// @pre <code>IsLocked()</code> returns false.
    /// @pre <code>IsStepComplete()</code> returns true.
    RegStepStats SolveReg(const StepConf& conf);

    /// @brief Solves the given island (regularly).
    ///
    /// @details This:
    ///   1. Updates every island-body's <code>sweep.pos0</code> to its <code>sweep.pos1</code>.
    ///   2. Updates every island-body's <code>sweep.pos1</code> to the new normalized "solved"
    ///      position for it.
    ///   3. Updates every island-body's velocity to the new accelerated, dampened, and "solved"
    ///      velocity for it.
    ///   4. Synchronizes every island-body's transform (by updating it to transform one of the
    ///      body's sweep).
    ///   5. Reports to the listener (if non-null).
    ///
    /// @param conf Time step configuration information.
    /// @param island Island of bodies, contacts, and joints to solve for. Must contain at least
    ///   one body, contact, or joint.
    ///
    /// @warning Behavior is undefined if the given island doesn't have at least one body,
    ///   contact, or joint.
    ///
    /// @return Island solver results.
    ///
    IslandStats SolveRegIslandViaGS(const StepConf& conf, const Island& island);
    
    /// @brief Adds to the island based off of a given "seed" body.
    /// @post Contacts are listed in the island in the order that bodies provide those contacts.
    /// @post Joints are listed the island in the order that bodies provide those joints.
    void AddToIsland(Island& island, BodyID seed,
                     BodyCounter& remNumBodies,
                     ContactCounter& remNumContacts,
                     JointCounter& remNumJoints);

    /// @brief Body stack.
    using BodyStack = std::vector<BodyID, pmr::polymorphic_allocator<BodyID>>;

    /// @brief Adds to the island.
    void AddToIsland(Island& island, BodyStack& stack,
                     BodyCounter& remNumBodies,
                     ContactCounter& remNumContacts,
                     JointCounter& remNumJoints);

    /// @brief Adds contacts of the specified body to the island and adds the other contacted
    ///   bodies to the body stack.
    void AddContactsToIsland(Island& island, BodyStack& stack, const BodyContacts& contacts,
                             BodyID bodyID);

    /// @brief Adds joints to the island.
    void AddJointsToIsland(Island& island, BodyStack& stack, const BodyJoints& joints);

    /// @brief Solves the step using successive time of impact (TOI) events.
    /// @details Used for continuous physics.
    /// @note This is intended to detect and prevent the tunneling that the faster Solve method
    ///    may miss.
    /// @param conf Time step configuration to use.
    ToiStepStats SolveToi(const StepConf& conf);

    /// @brief Solves collisions for the given time of impact.
    ///
    /// @param contactID Identifier of contact to solve for.
    /// @param conf Time step configuration to solve for.
    ///
    /// @note Precondition 1: there is no contact having a lower TOI in this time step that has
    ///   not already been solved for.
    /// @note Precondition 2: there is not a lower TOI in the time step for which collisions have
    ///   not already been processed.
    ///
    IslandStats SolveToi(ContactID contactID, const StepConf& conf);

    /// @brief Solves the time of impact for bodies 0 and 1 of the given island.
    ///
    /// @details This:
    ///   1. Updates position 0 of the sweeps of bodies 0 and 1.
    ///   2. Updates position 1 of the sweeps, the transforms, and the velocities of the other
    ///      bodies in this island.
    ///
    /// @pre <code>island.bodies</code> contains at least two bodies, the first two of which
    ///   are bodies 0 and 1.
    /// @pre <code>island.bodies</code> contains appropriate other bodies of the contacts of
    ///   the two bodies.
    /// @pre <code>island.contacts</code> contains the contact that specified the two identified
    ///   bodies.
    /// @pre <code>island.contacts</code> contains appropriate other contacts of the two bodies.
    ///
    /// @param conf Time step configuration information.
    /// @param island Island to do time of impact solving for.
    ///
    /// @return Island solver results.
    ///
    IslandStats SolveToiViaGS(const Island& island, const StepConf& conf);

    /// @brief Process contacts output.
    struct ProcessContactsOutput
    {
        ContactCounter contactsUpdated = 0; ///< Contacts updated.
        ContactCounter contactsSkipped = 0; ///< Contacts skipped.
    };

    /// @brief Processes the contacts of a given body for TOI handling.
    /// @details This does the following:
    ///   1. Advances the appropriate associated other bodies to the given TOI (advancing
    ///      their sweeps and synchronizing their transforms to their new sweeps).
    ///   2. Updates the contact manifolds and touching statuses and notifies listener (if one given) of
    ///      the appropriate contacts of the body.
    ///   3. Adds those contacts that are still enabled and still touching to the given island
    ///      (or resets the other bodies advancement).
    ///   4. Adds to the island, those other bodies that haven't already been added of the contacts that got added.
    /// @note Precondition: there should be no lower TOI for which contacts have not already been processed.
    /// @param[in,out] id Identifier of the dynamic/accelerable body to process contacts for.
    /// @param[in,out] island Island. On return this may contain additional contacts or bodies.
    /// @param[in] toi Time of impact (TOI). Value between 0 and 1.
    /// @param[in] conf Step configuration data.
    ProcessContactsOutput ProcessContactsForTOI(BodyID id, Island& island, Real toi,
                                                const StepConf& conf);

    /// @brief Removes the given body from this world.
    void Remove(BodyID id);

    /// @brief Updates associated bodies and contacts for specified joint's addition.
    void Add(JointID id, bool flagForFiltering = false);

    /// @brief Updates associated bodies and contacts for specified joint's removal.
    void Remove(JointID id);

    /// @brief Update contacts statistics.
    struct UpdateContactsStats
    {
        /// @brief Number of contacts ignored (because both bodies were asleep).
        ContactCounter ignored = 0;

        /// @brief Number of contacts updated.
        ContactCounter updated = 0;

        /// @brief Number of contacts skipped because they weren't marked as needing updating.
        ContactCounter skipped = 0;
    };

    /// @brief Destroy contacts statistics.
    struct DestroyContactsStats
    {
        ContactCounter overlap = 0; ///< Erased by not overlapping.
        ContactCounter filter = 0; ///< Erased due to filtering.
    };

    /// @brief Update contacts data.
    struct UpdateContactsData
    {
        ContactCounter numAtMaxSubSteps = 0; ///< # at max sub-steps (lower the better).
        ContactCounter numUpdatedTOI = 0; ///< # updated TOIs (made valid).
        ContactCounter numValidTOI = 0; ///< # already valid TOIs.

        /// @brief Distance iterations type alias.
        using dist_iter_type = std::remove_const<decltype(DefaultMaxDistanceIters)>::type;

        /// @brief TOI iterations type alias.
        using toi_iter_type = std::remove_const<decltype(DefaultMaxToiIters)>::type;

        /// @brief Root iterations type alias.
        using root_iter_type = std::remove_const<decltype(DefaultMaxToiRootIters)>::type;

        dist_iter_type maxDistIters = 0; ///< Max distance iterations.
        toi_iter_type maxToiIters = 0; ///< Max TOI iterations.
        root_iter_type maxRootIters = 0; ///< Max root iterations.
    };

    struct Listeners
    {
        ShapeListener shapeDestruction; ///< Listener for shape destruction.
        AssociationListener detach; ///< Listener for shapes detaching from bodies.
        JointListener jointDestruction; ///< Listener for joint destruction.
        ContactListener beginContact; ///< Listener for beginning contact events.
        ContactListener endContact; ///< Listener for ending contact events.
        ManifoldContactListener preSolveContact; ///< Listener for pre-solving contacts.
        ImpulsesContactListener postSolveContact; ///< Listener for post-solving contacts.
    };

    /// @brief Updates the contact times of impact.
    UpdateContactsData UpdateContactTOIs(const StepConf& conf);

    /// @brief Processes the narrow phase collision for the contacts collection.
    /// @details
    /// This finds and destroys the contacts that need filtering and no longer should collide or
    /// that no longer have AABB-based overlapping fixtures. Those contacts that persist and
    /// have active bodies (either or both) get their Update methods called with the current
    /// contact listener as its argument.
    /// Essentially this really just purges contacts that are no longer relevant.
    DestroyContactsStats DestroyContacts(Contacts& contacts);
    
    /// @brief Update contacts.
    UpdateContactsStats UpdateContacts(const StepConf& conf);

    /// @brief Adds new contacts.
    /// @details Processes the proxy queue for finding new contacts and adding them to
    ///   the contacts container.
    /// @note New contacts will all have overlapping AABBs.
    /// @post <code>GetProxies()</code> will return an empty container.
    /// @post Container returned by <code>GetContacts()</code> will have increased in size by returned amount.
    /// @post Container returned by <code>GetContacts(BodyID)</code> for some body IDs may have more elements.
    /// @see GetProxies.
    ContactCounter AddNewContacts(std::vector<ProxyKey, pmr::polymorphic_allocator<ProxyKey>>&& contactKeys);

    /// @brief Destroys the given contact and removes it from its container.
    /// @details This updates the contacts container, returns the memory to the allocator,
    ///   and decrements the contact manager's contact count.
    /// @param contact Contact to destroy.
    /// @param from From body.
    void Destroy(ContactID contact, const Body* from);

    /// @brief Destroys the given contact.
    void InternalDestroy(ContactID contact, const Body* from = nullptr);

    /// @brief Synchronizes the given body.
    /// @details This updates the broad phase dynamic tree data for all of the identified shapes.
    ContactCounter Synchronize(const Proxies& bodyProxies,
                               const Transformation& xfm0, const Transformation& xfm1,
                               Real multiplier, Length extension);

    /// @brief Updates the touching related state and notifies listener (if one given).
    ///
    /// @note Ideally this method is only called when a dependent change has occurred.
    /// @note Touching related state depends on the following data:
    ///   - The fixtures' sensor states.
    ///   - The fixtures bodies' transformations.
    ///   - The <code>maxCirclesRatio</code> per-step configuration state *OR* the
    ///     <code>maxDistanceIters</code> per-step configuration state.
    ///
    /// @param id Identifies the contact to update.
    /// @param conf Per-step configuration information.
    ///
    /// @see GetManifold, IsTouching
    ///
    void Update(ContactID id, const ContactUpdateConf& conf);

    /******** Member variables. ********/

    pmr::PoolMemoryResource m_bodyStackResource;
    pmr::PoolMemoryResource m_bodyConstraintsResource;
    pmr::PoolMemoryResource m_positionConstraintsResource;
    pmr::PoolMemoryResource m_velocityConstraintsResource;
    pmr::PoolMemoryResource m_proxyKeysResource;
    pmr::PoolMemoryResource m_islandResource;

    DynamicTree m_tree; ///< Dynamic tree.

    ObjectPool<Body> m_bodyBuffer; ///< Array of body data both used and freed.
    ObjectPool<Shape> m_shapeBuffer; ///< Array of shape data both used and freed.
    ObjectPool<Joint> m_jointBuffer; ///< Array of joint data both used and freed.

    /// @brief Array of contact data both used and freed.
    ObjectPool<Contact> m_contactBuffer;

    /// @brief Array of manifold data both used and freed.
    /// @note Size depends on and matches <code>size(m_contactBuffer)</code>.
    ObjectPool<Manifold> m_manifoldBuffer;

    /// @brief Cache of contacts associated with bodies.
    /// @note Size depends on and matches <code>size(m_bodyBuffer)</code>.
    /// @note Individual body contact containers are added to by <code>AddNewContacts</code>.
    ObjectPool<BodyContacts> m_bodyContacts;

    ///< Cache of joints associated with bodies.
    /// @note Size depends on and matches <code>size(m_bodyBuffer)</code>.
    ObjectPool<BodyJoints> m_bodyJoints;

    /// @brief Cache of proxies associated with bodies.
    /// @note Size depends on and matches <code>size(m_bodyBuffer)</code>.
    ObjectPool<Proxies> m_bodyProxies;

    /// @brief Buffer of proxies to inspect for finding new contacts.
    /// @note Built from @a m_fixturesForProxies and on body synchronization. Consumed by the finding-of-new-contacts.
    Proxies m_proxiesForContacts;

    /// @brief Fixtures for proxies queue.
    /// @note Capacity grows on calls to <code>CreateBody</code>, <code>SetBody</code>, and <code>SetShape</code>.
    std::vector<std::pair<BodyID, ShapeID>> m_fixturesForProxies;

    /// @brief Bodies for proxies queue.
    /// @note Size & capacity grows on calls to <code>SetBody</code>.
    /// @note Size shrinks on calls to <code>Remove(BodyID id)</code>.
    /// @note Size clears on calls to <code>Step</code> or <code>Clear</code>.
    Bodies m_bodiesForSync;

    Bodies m_bodies; ///< Body collection.

    Joints m_joints; ///< Joint collection.
    
    /// @brief Container of contacts.
    /// @note In the <em>add pair</em> stress-test, 401 bodies can have some 31000 contacts
    ///   during a given time step.
    Contacts m_contacts;

    /// Bodies, contacts, and joints that are already in an island.
    /// @note This is step-wise state that needs to be here or within a step solving co-routine for sub-stepping TOI solving.
    /// @note This instance's members capacities depend on state changed outside the step loop.
    /// @see Island.
    Islanded m_islanded;

    /// @brief Listeners.
    Listeners m_listeners;

    FlagsType m_flags = e_stepComplete; ///< Flags.
    
    /// Inverse delta-t from previous step.
    /// @details Used to compute time step ratio to support a variable time step.
    /// @note 4-bytes large.
    /// @see Step.
    Frequency m_inv_dt0 = 0_Hz;

    /// @brief Min and max vertex radii.
    /// @details
    /// The interval max is the maximum shape vertex radius that any bodies' of this world should
    /// create fixtures for. Requests to create fixtures for shapes with vertex radiuses bigger than
    /// this must be rejected. As an upper bound, this value prevents shapes from getting
    /// associated with this world that would otherwise not be able to be simulated due to
    /// numerical issues. It can also be set below this upper bound to constrain the differences
    /// between shape vertex radiuses to possibly more limited visual ranges.
    Interval<Positive<Length>> m_vertexRadius{WorldConf::DefaultMinVertexRadius, WorldConf::DefaultMaxVertexRadius};
};

inline const WorldImpl::Proxies& WorldImpl::GetProxies() const noexcept
{
    return m_proxiesForContacts;
}

inline const WorldImpl::Bodies& WorldImpl::GetBodies() const noexcept
{
    return m_bodies;
}

inline const WorldImpl::Bodies& WorldImpl::GetBodiesForProxies() const noexcept
{
    return m_bodiesForSync;
}

inline const std::vector<std::pair<BodyID, ShapeID>>& WorldImpl::GetFixturesForProxies() const noexcept
{
    return m_fixturesForProxies;
}

inline const WorldImpl::Joints& WorldImpl::GetJoints() const noexcept
{
    return m_joints;
}

inline std::vector<KeyedContactID> WorldImpl::GetContacts() const
{
    return std::vector<KeyedContactID>{begin(m_contacts), end(m_contacts)};
}

inline bool WorldImpl::IsLocked() const noexcept
{
    return (m_flags & e_locked) == e_locked;
}

inline bool WorldImpl::IsStepComplete() const noexcept
{
    return (m_flags & e_stepComplete) != 0u;
}

inline bool WorldImpl::GetSubStepping() const noexcept
{
    return (m_flags & e_substepping) != 0u;
}

inline void WorldImpl::SetSubStepping(bool flag) noexcept
{
    if (flag) {
        m_flags |= e_substepping;
    }
    else {
        m_flags &= ~e_substepping;
    }
}

inline Length WorldImpl::GetMinVertexRadius() const noexcept
{
    return m_vertexRadius.GetMin();
}

inline Length WorldImpl::GetMaxVertexRadius() const noexcept
{
    return m_vertexRadius.GetMax();
}

inline Frequency WorldImpl::GetInvDeltaTime() const noexcept
{
    return m_inv_dt0;
}

inline const DynamicTree& WorldImpl::GetTree() const noexcept
{
    return m_tree;
}

inline void WorldImpl::SetShapeDestructionListener(ShapeListener listener) noexcept
{
    m_listeners.shapeDestruction = std::move(listener);
}

inline void WorldImpl::SetDetachListener(AssociationListener listener) noexcept
{
    m_listeners.detach = std::move(listener);
}

inline void WorldImpl::SetJointDestructionListener(JointListener listener) noexcept
{
    m_listeners.jointDestruction = std::move(listener);
}

inline void WorldImpl::SetBeginContactListener(ContactListener listener) noexcept
{
    m_listeners.beginContact = std::move(listener);
}

inline void WorldImpl::SetEndContactListener(ContactListener listener) noexcept
{
    m_listeners.endContact = std::move(listener);
}

inline void WorldImpl::SetPreSolveContactListener(ManifoldContactListener listener) noexcept
{
    m_listeners.preSolveContact = std::move(listener);
}

inline void WorldImpl::SetPostSolveContactListener(ImpulsesContactListener listener) noexcept
{
    m_listeners.postSolveContact = std::move(listener);
}

// State & confirm compile-time traits of WorldImpl class.
// It minimally needs to be default and copy constructable.
static_assert(std::is_default_constructible_v<WorldImpl>);
static_assert(std::is_copy_constructible_v<WorldImpl>);
static_assert(!std::is_move_constructible_v<WorldImpl>);
static_assert(!std::is_copy_assignable_v<WorldImpl>);
static_assert(!std::is_move_assignable_v<WorldImpl>);

} // namespace d2
} // namespace playrho

#endif // PLAYRHO_DYNAMICS_WORLDIMPL_HPP
