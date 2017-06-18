#pragma once
#include "glm.hpp"

//Collision manager, algorithm and plane classes referenced from Soren Seeberg
//http://www.peroxide.dk/papers/collision/collision.pdf

using namespace glm;
using namespace std;

//Barycentric technique referenced from http://blackpawn.com/texts/pointinpoly/
bool checkPointInTriangle(const vec3 &point, const vec3 &p1, const vec3 &p2, const vec3 & p3)
{
	vec3 v0 = p2 - p1;
	vec3 v1 = p3 - p1;
	vec3 v2 = point - p1;

	float dot00 = dot(v0, v0);
	float dot01 = dot(v0, v1);
	float dot02 = dot(v0, v2);
	float dot11 = dot(v1, v1);
	float dot12 = dot(v1, v2);

	//barycentric coordinates
	float denumerator = dot00 * dot11 - dot01 * dot01;
	float u = (dot11 * dot02 - dot01 * dot12) / denumerator;
	float v = (dot00 * dot12 - dot01 * dot02) / denumerator;

	return (u >= 0) && (v >= 0) && (u + v < 1);
}

//Solution of quadratic equation references Soren Seedberg
bool getLowestRoot(float a, float b, float c, float current, float* root)
{
	float determinant = b*b - 4.0f * a * c;
	//Check for complex solutions
	if (determinant < 0)
	{
		return false;
	}

	//calculate roots
	float rootD = sqrt(determinant);
	float r1 = (-b - rootD) / 2 * a;
	float r2 = (-b + rootD) / 2 * a;

	//Sort roots
	if (r1 > r2)
	{
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}
	//We're looking for the lowest value between [0,1];
	if (r1 > 0 && r1 < current)
	{
		*root = r1;
		return true;
	}

	//According to reference, this can happen if x1 negative
	if (r2 > 0 && r2 < current)
	{
		*root = r2;
		return true;
	}

	return false;
}

struct CollisionPacket
{
	vec3 elipsoidRadius;

	vec3 R3velocity;
	vec3 R3position;

	vec3 eVelocity;
	vec3 eNormalizedVelocity;
	vec3 eBasePoint;

	bool foundCollision;
	double nearestDistance;
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
		normal = cross((p2 - p1),(p3 - p1));
		normal = normalize(normal);
		origin = p1;

		equation[0] = normal.x;
		equation[1] = normal.y;
		equation[2] = normal.z;
		//Equation of a plane
		equation[3] = -(normal.x*origin.x + normal.y*origin.y + normal.z*origin.z);
	}

	//Like in the shaders
	bool isFrontFacingTo(const vec3 &direction) const
	{
		return max(dot(normal, direction), 0.0f);
	}
	double signedDistanceTo(const vec3 &point) const
	{
		return dot(point, normal) + equation[3];
	}
};

class CollisionManager
{
	static CollisionManager* instance;
	
public:
	static CollisionManager *getInstance()
	{
		if (!instance)
		{
			instance = new CollisionManager;
		}
		return instance;
	}

	bool askMove(glm::vec3 velocity, glm::vec3 worldPosition)
	{
		//Here we want to verify whether the move will cause a collision
		return true;
	}

	//Assume that p1, p2 and p3 are given in front-facing order
	void checkTriangle(CollisionPacket* col, vec3 p1, vec3 p2, vec3 p3)
	{
		Plane triangle(p1, p2, p3);

		//Proceed only if triangle is front-facing to normal
		if (triangle.isFrontFacingTo(col->eNormalizedVelocity))
		{
			float t0, t1;
			bool embeddedInPlane = false;

			//Calculate distance from sphere to plane
			double distToPlane = triangle.signedDistanceTo(col->eBasePoint);
			float normalDotVelocity = dot(triangle.normal, col->eVelocity);

			//special case where normal is perpendicular (sphere traveling parallel to plane)
			if (normalDotVelocity == 0.0f)
			{
				if (abs(distToPlane) >= 1.0f)
				{
					//no collision
					return;
				}
				else
				{
					//sphere embedded in plane
					embeddedInPlane = true;
					//sphere interesects plane at all times
					t0 = 0.0;
					t1 = 1.0;
				}
			}
			//non-parallel, calculate intersection
			else
			{
				t0 = (-1.0 - distToPlane) / normalDotVelocity;
				t1 = (1.0 - distToPlane) / normalDotVelocity;
				//make sure t0 is the smaller distance
				if (t0 > t1)
				{
					float temp = t1;
					t1 = t0;
					t0 = temp;
				}
			}

			//Check for collisions within the sphere's radius of 1.0f
			if (t0 > 1.0f || t1 < 0.0f)
			{
				//Outside the radius, no collision
				return;
			}

			if (t0 < 0.0)
			{
				t0 = 0.0;
			}
			if (t1 < 0.0)
			{
				t1 = 0.0;
			}
			if (t0 > 1.0)
			{
				t0 = 1.0;
			}
			if (t1 > 1.0)
			{
				t1 = 1.0;
			}

			//If a collision happens it's between these two values
			vec3 collisionPoint;
			bool foundCollision = false;
			float t = 1.0;

			//Check collision inside the triangle
			if (!embeddedInPlane)
			{
				vec3 planeIntersectionPoint = (col->eBasePoint - triangle.normal) + t0 * col->eVelocity;
				if (checkPointInTriangle(planeIntersectionPoint, p1, p2, p3))
				{
					//If collision happened inside the triangle it must have been at t0 (explained in referenced paper)
					foundCollision = true;
					t = t0;
					collisionPoint = planeIntersectionPoint;
				}
			}

			//If collision wasn't inside the triangle, proceed with the expensive sweep method for edge/vertex collisions
			//A triangle collision will always occur before an edge or vertex collision
			if (!foundCollision)
			{
				vec3 velocity = col->eVelocity;
				vec3 base = col->eBasePoint;
				float velocitySquaredLength = dot(velocity, velocity);
				float a, b, c;
				float newT;

				//As explained in the paper (Equation 3.5), we find the time of intersection by solving the
				//quadratic equation At^2 + Bt + C, where:
				//A = dot(velocity, velocity) (velocitySquaredLength, calculated above)
				//B = 2 * dot(velocity, basePoint - p)
				//C = dot(p - basePoint, p - basePoint) - 1
				// p is each of the points to be checked (p1, p2, p3)
				a = velocitySquaredLength;

				//Check p1
				b = 2.0f * dot(velocity, base - p1);
				c = dot(p1 - base, p1 - base) - 1.0;
				if (getLowestRoot(a, b, c, t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = p1;
				}

				//Check p2
				b = 2.0f * dot(velocity, base - p2);
				c = dot(p2 - base, p2 - base) - 1.0;
				if (getLowestRoot(a, b, c, t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = p2;
				}

				//Check p3
				b = 2.0f * dot(velocity, base - p3);
				c = dot(p3 - base, p3 - base) - 1.0;
				if (getLowestRoot(a, b, c, t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = p3;
				}

				//According to above reference, even if an edge collision is true
				//We must still check against edges, as it's possible that an
				//edge collision happens sooner

				//p1 - p2 edge
				vec3 edge = p2 - p1;
				vec3 baseToVertex = p1 - base;

				//Calculate a,b,c, as described on page 16
				a = dot(edge, edge) * -velocitySquaredLength + dot(edge, velocity) * dot(edge, velocity);
				b = dot(edge, edge) * 2 * dot(velocity, baseToVertex) - 2 * (dot(edge, velocity) * dot(edge, baseToVertex));
				c = dot(edge, edge) * (1 - dot(baseToVertex, baseToVertex)) + (dot(edge, baseToVertex) * dot(edge, baseToVertex));

				if (getLowestRoot(a, b, c, t, &newT))
				{
					//Calculate where along the edge (rather, its infinity line) the intersection happens (Equation 3.6)
					float f = (dot(edge, velocity) * newT - dot(edge, baseToVertex)) / (dot(edge, edge));
					//Since we calculated roots along the infinity line, check if it lies on the triangle edge
					if (f >= 0.0 && f <= 1.0)
					{
						t = newT;
						foundCollision = true;
						collisionPoint = p1 + f*edge; 
					}
				}

				//Likewise for all other edges
				//p2 - p3 edge
				edge = p3-p2;
				baseToVertex = p2 - base;

				a = dot(edge, edge) * -velocitySquaredLength + dot(edge, velocity) * dot(edge, velocity);
				b = dot(edge, edge) * 2 * dot(velocity, baseToVertex) - 2 * (dot(edge, velocity) * dot(edge, baseToVertex));
				c = dot(edge, edge) * (1 - dot(baseToVertex, baseToVertex)) + (dot(edge, baseToVertex) * dot(edge, baseToVertex));

				if (getLowestRoot(a, b, c, t, &newT))
				{
					float f = (dot(edge, velocity) * newT - dot(edge, baseToVertex)) / (dot(edge, edge));
					if (f >= 0.0 && f <= 1.0)
					{
						t = newT;
						foundCollision = true;
						collisionPoint = p2 + f*edge;
					}
				}

				//p3 - p1 edge
				edge = p1 - p3;
				baseToVertex = p3 - base;

				a = dot(edge, edge) * -velocitySquaredLength + dot(edge, velocity) * dot(edge, velocity);
				b = dot(edge, edge) * 2 * dot(velocity, baseToVertex) - 2 * (dot(edge, velocity) * dot(edge, baseToVertex));
				c = dot(edge, edge) * (1 - dot(baseToVertex, baseToVertex)) + (dot(edge, baseToVertex) * dot(edge, baseToVertex));

				if (getLowestRoot(a, b, c, t, &newT))
				{
					float f = (dot(edge, velocity) * newT - dot(edge, baseToVertex)) / (dot(edge, edge));
					if (f >= 0.0 && f <= 1.0)
					{
						t = newT;
						foundCollision = true;
						collisionPoint = p3 + f*edge;
					}
				}

			}

			if (foundCollision == true)
			{
				//set distance to collision
				float distToCollision = t*length(col->eVelocity);

				//This is the closest hit if its the first or closest
				if (col->foundCollision == false || distToCollision < col->nearestDistance)
				{
					col->nearestDistance = distToCollision;
					col->intersectionPoint = collisionPoint;
					col->foundCollision = true;
				}
			}
		}
	}
};