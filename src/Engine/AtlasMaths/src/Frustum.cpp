#include "AtlasMathsPCH.h"
#include "AtlasMaths/Frustum.h"

namespace
{
    enum class PlaneSide
    {
        Positive,
        Negative,
        Both
    };

    float getDistance(const bx::Plane& plane, const Eigen::Vector3f& point)
    {
        const Eigen::Vector3f normal { plane.normal.x, plane.normal.y, plane.normal.z };
        return normal.dot(point) + plane.dist;
    }

    PlaneSide getSide(const bx::Plane& plane, const Eigen::Vector3f& point)
    {
        const float distance = getDistance(plane, point);
        if (distance > 0.0f)
        {
            return PlaneSide::Positive;
        }

        if (distance < 0.0f)
        {
            return PlaneSide::Negative;
        }

        return PlaneSide::Both;
    }

    Eigen::Vector3f cast(const bx::Vec3& vec)
    {
        return Eigen::Vector3f { vec.x, vec.y, vec.z };
    }
}

using namespace atlas::maths;

Frustum::Frustum()
	: mPlanes{
	    { bx::init::None, bx::init::None, bx::init::None, bx::init::None, bx::init::None, bx::init::None }
	}
{
}

Frustum::Frustum(const Eigen::Matrix4f& viewProjection)
	: Frustum()
{
    Build(viewProjection);
}

void Frustum::Build(const Eigen::Matrix4f& viewProjection)
{
    buildFrustumPlanes(mPlanes.data(), viewProjection.data());
    mCorners[Corners::FTL] = cast(bx::intersectPlanes(mPlanes[Far], mPlanes[Top], mPlanes[Left]));
    mCorners[Corners::FTR] = cast(bx::intersectPlanes(mPlanes[Far], mPlanes[Top], mPlanes[Right]));
    mCorners[Corners::FBL] = cast(bx::intersectPlanes(mPlanes[Far], mPlanes[Bottom], mPlanes[Left]));
    mCorners[Corners::FBR] = cast(bx::intersectPlanes(mPlanes[Far], mPlanes[Bottom], mPlanes[Right]));
    mCorners[Corners::NTL] = cast(bx::intersectPlanes(mPlanes[Near], mPlanes[Top], mPlanes[Left]));
    mCorners[Corners::NTR] = cast(bx::intersectPlanes(mPlanes[Near], mPlanes[Top], mPlanes[Right]));
    mCorners[Corners::NBL] = cast(bx::intersectPlanes(mPlanes[Near], mPlanes[Bottom], mPlanes[Left]));
    mCorners[Corners::NBR] = cast(bx::intersectPlanes(mPlanes[Near], mPlanes[Bottom], mPlanes[Right]));
}

const bx::Plane& Frustum::GetPlane(uint8_t index) const
{
	return mPlanes[index];
}

const bx::Plane& Frustum::GetPlane(Planes plane) const
{
	return mPlanes[static_cast<uint32_t>(plane)];
}

const Eigen::Vector3f& Frustum::GetCorner(uint8_t index) const
{
	return mCorners[index];
}

const Eigen::Vector3f& Frustum::GetCorner(Corners corner) const
{
	return mCorners[static_cast<uint32_t>(corner)];
}

