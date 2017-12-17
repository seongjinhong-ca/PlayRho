/*
 * Original work Copyright (c) 2007-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_COLLISION_SEPARATIONFINDER_HPP
#define PLAYRHO_COLLISION_SEPARATIONFINDER_HPP

#include <PlayRho/Common/Math.hpp>
#include <PlayRho/Collision/IndexPair.hpp>

namespace playrho {

    class DistanceProxy;
    struct Transformation2D;
        
    /// Separation finder.
    class SeparationFinder
    {
    public:
        
        /// Separation finder type.
        enum Type
        {
            e_points,
            e_faceA,
            e_faceB
        };
        
        /// Gets a separation finder for the given inputs.
        ///
        /// @warning Behavior is undefined if given less than one index pair or more than three.
        ///
        /// @param indices Collection of 1 to 3 index pairs. A points-type finder will be
        ///    returned if given 1 index pair. A face-type finder will be returned otherwise.
        /// @param proxyA Proxy A.
        /// @param xfA Transformation A.
        /// @param proxyB Proxy B.
        /// @param xfB Transformation B.
        ///
        static SeparationFinder Get(IndexPair3 indices,
                                    const DistanceProxy& proxyA, const Transformation2D& xfA,
                                    const DistanceProxy& proxyB, const Transformation2D& xfB);
        
        /// Finds the minimum separation.
        /// @return indexes of proxy A's and proxy B's vertices that have the minimum
        ///    distance between them and what that distance is.
        LengthIndexPair FindMinSeparation(const Transformation2D& xfA,
                                            const Transformation2D& xfB) const
        {
            switch (m_type)
            {
                case e_points: return FindMinSeparationForPoints(xfA, xfB);
                case e_faceA: return FindMinSeparationForFaceA(xfA, xfB);
                case e_faceB: return FindMinSeparationForFaceB(xfA, xfB);
            }
            
            // Should never be reached
            assert(false);
            return LengthIndexPair{0, InvalidIndexPair};
        }
        
        /// Evaluates the separation of the identified proxy vertices at the given time factor.
        ///
        /// @param indexPair Indexes of the proxy A and proxy B vertexes.
        /// @param xfA Transformation A.
        /// @param xfB Transformation B.
        ///
        /// @return Separation distance which will be negative when the given transforms put the
        ///    vertices on the opposite sides of the separating axis.
        ///
        Length Evaluate(const Transformation2D& xfA, const Transformation2D& xfB,
                        IndexPair indexPair) const
        {
            switch (m_type)
            {
                case e_points: return EvaluateForPoints(xfA, xfB, indexPair);
                case e_faceA: return EvaluateForFaceA(xfA, xfB, indexPair);
                case e_faceB: return EvaluateForFaceB(xfA, xfB, indexPair);
                default: break;
            }
            assert(false);
            return 0_m;
        }
        
        /// @brief Gets the type.
        PLAYRHO_CONSTEXPR inline Type GetType() const noexcept;

        /// @brief Gets the axis.
        PLAYRHO_CONSTEXPR inline UnitVec2 GetAxis() const noexcept;
        
        /// @brief Gets the local point.
        PLAYRHO_CONSTEXPR inline Length2 GetLocalPoint() const noexcept;

    private:
        
        /// @brief Initializing constructor.
        PLAYRHO_CONSTEXPR inline SeparationFinder(const DistanceProxy& dpA, const DistanceProxy& dpB,
                                         const UnitVec2 axis, const Length2 lp, const Type type):
            m_proxyA{dpA}, m_proxyB{dpB}, m_axis{axis}, m_localPoint{lp}, m_type{type}
        {
            // Intentionally empty.
        }
        
        /// @brief Finds the minimum separation for points.
        LengthIndexPair FindMinSeparationForPoints(const Transformation2D& xfA,
                                                     const Transformation2D& xfB) const;
        
        /// @brief Finds the minimum separation for face A.
        LengthIndexPair FindMinSeparationForFaceA(const Transformation2D& xfA,
                                                    const Transformation2D& xfB) const;
        
        /// @brief Finds the minimum separation for face B.
        LengthIndexPair FindMinSeparationForFaceB(const Transformation2D& xfA,
                                                    const Transformation2D& xfB) const;
        
        /// @brief Evaluates for points.
        Length EvaluateForPoints(const Transformation2D& xfA, const Transformation2D& xfB,
                                 IndexPair indexPair) const;
        
        /// @brief Evaluates for face A.
        Length EvaluateForFaceA(const Transformation2D& xfA, const Transformation2D& xfB,
                                IndexPair indexPair) const;
        
        /// @brief Evaluates for face B.
        Length EvaluateForFaceB(const Transformation2D& xfA, const Transformation2D& xfB,
                                IndexPair indexPair) const;
        
        const DistanceProxy& m_proxyA; ///< Distance proxy A.
        const DistanceProxy& m_proxyB; ///< Distance proxy B.
        const UnitVec2 m_axis; ///< Axis. @details Directional vector of the axis of separation.
        const Length2 m_localPoint; ///< Local point. @note Only used if type is e_faceA or e_faceB.
        const Type m_type; ///< The type of this instance.
    };

    PLAYRHO_CONSTEXPR inline SeparationFinder::Type SeparationFinder::GetType() const noexcept
    {
        return m_type;
    }
    
    PLAYRHO_CONSTEXPR inline UnitVec2 SeparationFinder::GetAxis() const noexcept
    {
        return m_axis;
    }
    
    PLAYRHO_CONSTEXPR inline Length2 SeparationFinder::GetLocalPoint() const noexcept
    {
        return m_localPoint;
    }

} // namespace playrho

#endif // PLAYRHO_COLLISION_SEPARATIONFINDER_HPP
