#pragma once
#include "glm.hpp"

using namespace glm;

struct CollisionPacket
{
	vec3 elipsoidRadius;

	vec3 R3velocity;
	vec3 R3position;

	vec3 eVelocity;
	vec3 eNormalizedVelocity;
	vec3 eBasePoint;

	bool foundCollision;
	float nearestDistance;
	vec3 intersectionPoint;
};

class Plane
{
public:
	float equation[4];
	vec3 origin;
	vec3 normal;

	Plane(const vec3 &origin, const vec3 &normal)
	{
		this->normal = normal;
		this->origin = origin;
		equation[0] = normal.x;
		equation[1] = normal.y;
		equation[2] = normal.z;
		//Equation of the plane
		equation[3] = -(normal.x*origin.x + normal.y*origin.y + normal.z*origin.z);
	}
	Plane(const vec3 &p1, const vec3 &p2, const vec3 &p3)
	{
		normal = cross((p2 - p1), (p3 - p1));
		normal = normalize(normal);
		origin = p1;

		equation[0] = normal.x;
		equation[1] = normal.y;
		equation[2] = normal.z;
		//Equation of a plane
		equation[3] = -(normal.x*origin.x + normal.y*origin.y + normal.z*origin.z);
	}

	//Like in the shaders
	inline bool isFrontFacingTo(const vec3 &direction) const
	{
		float dotproduct = dot(normal, direction);
		return glm::max(dotproduct, 0.0f);
	}
	inline float signedDistanceTo(const vec3 &point) const
	{
		return dot(point, normal) + equation[3];
	}
};