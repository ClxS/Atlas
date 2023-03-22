#pragma once
#include "bx/bounds.h"
#include "bx/math.h"
#include "Eigen/Core"

namespace atlas::maths
{
    class Frustum
    {
    public:
        enum Planes
        {
            Far = 0,
            Near = 1,
            Left = 2,
            Right = 3,
            Bottom = 4,
            Top = 5
            /*Left, , Bottom, Top, Right, Left*/
        };
        enum Corners
        {
            FTL = 0, FTR, FBR, FBL, NTL, NTR, NBR, NBL
        };

        Frustum();
        explicit Frustum(const Eigen::Matrix4f& viewProjection);

        void Build(const Eigen::Matrix4f& viewProjection);

        // TODO Add intersection/contains tests

        const bx::Plane& GetPlane(uint8_t index) const;
        const bx::Plane& GetPlane(Planes plane) const;

        const Eigen::Vector3f& GetCorner(uint8_t index) const;
        const Eigen::Vector3f& GetCorner(Corners corner) const;

    private:
        std::array<bx::Plane, 6> mPlanes;
        Eigen::Vector3f mCorners[8];
    };
}
