/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
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

#include <cmath>

#include <PlayRho/Collision/ClipList.hpp>

namespace playrho::d2 {

ClipList ClipSegmentToLine(const ClipList& vIn, const UnitVec& normal, Length offset,
                           ContactFeature::Index indexA)
{
    ClipList vOut;

    if (size(vIn) == 2) // must have two points (for a segment)
    {
        // Use Sutherland-Hodgman clipping:
        //   (https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm ).

        // Calculate the distance of end points to the line
        const auto distance0 = Dot(normal, vIn[0].v) - offset;
        const auto distance1 = Dot(normal, vIn[1].v) - offset;

        // If the points are behind the plane...
        // Ideally they are. Then we get face-vertex contact features which are simpler to
        // calculate. Note that it also helps to avoid changing the contact feature from the
        // given clip vertices. So the code here also accepts distances that are just slightly
        // over zero.
        if (distance0 <= 0_m || AlmostZero(StripUnit(distance0)))
        {
            vOut.push_back(vIn[0]);
        }
        if (distance1 <= 0_m || AlmostZero(StripUnit(distance1)))
        {
            vOut.push_back(vIn[1]);
        }

        // If we didn't already find two points & the points are on different sides of the plane...
        if (size(vOut) < 2 && signbit(StripUnit(distance0)) != signbit(StripUnit(distance1)))
        {
            // Neither distance0 nor distance1 is 0 and either one or the other is negative (but not both).
            // Find intersection point of edge and plane
            // Vertex A is hitting edge B.
            const auto interp = distance0 / (distance0 - distance1);
            const auto vertex = vIn[0].v + (vIn[1].v - vIn[0].v) * interp;
            vOut.push_back(ClipVertex{vertex, GetVertexFaceContactFeature(indexA, vIn[0].cf.indexB)});
        }
    }

    return vOut;
}

}
