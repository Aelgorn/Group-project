#include "CollisionManager.h"
#include "collision_math.h"
#include "model.h"

//Since this is a static class, definition is in a .cpp file
static CollisionManager* instance = 0;

CollisionManager* CollisionManager::getInstance()
{
	{
		if (!instance)
		{
			instance = new CollisionManager;
		}
		return instance;
	}
}

//Tracks the vertices bounding boxes of a model;
void CollisionManager::trackModel(Model* model)
{
	std::cout << "Tracking the added model" << std::endl;
	tracked_models.push_back(model);
}

//Solution of quadratic equation references Soren Seedberg
bool CollisionManager::getLowestRoot(float a, float b, float c, float current, float* root)
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

//Barycentric technique referenced from http://blackpawn.com/texts/pointinpoly/
bool CollisionManager::checkPointInTriangle(const vec3 &point, const vec3 &p1, const vec3 &p2, const vec3 & p3)
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

vec3 CollisionManager::askMove(glm::vec3 elipsoidradius, glm::vec3 R3velocity, glm::vec3 R3position)
{
	glm::vec3 result;
	//Construct a collision packet
	CollisionPacket packet;
	packet.elipsoidRadius = elipsoidradius;
	packet.R3velocity = R3velocity;
	packet.R3position = R3position;

	//Create a "Change of Basis Matrix" to translate to elipse space (equation 2.4)
	mat3 CBM = mat3(
		glm::vec3(1 / elipsoidradius.x, 0.0f, 0.0f),
		glm::vec3(0.0f, 1 / elipsoidradius.y, 0.0f),
		glm::vec3(0.0f, 0.0f, 1 / elipsoidradius.z)
	);

	//Calculate the elipse space vectors
	packet.eVelocity = CBM * R3velocity;
	packet.eNormalizedVelocity = normalize(packet.eVelocity);
	packet.eBasePoint = CBM * R3position;

	packet.foundCollision = false;

	//Attempt to process the collision for each bounding box triangle
	for (unsigned int i = 0; i < tracked_models.size(); i++)
	{
		std::vector<vector<vec3>> box = tracked_models[i]->getBoundingBoxes();
		//Front-facing triangles based on vertices as described in model.h
		for (int i = 0; i < box.size(); ++i){
		//Front face
		checkTriangle(&packet, box[i][3], box[i][1], box[i][0]);
		checkTriangle(&packet, box[i][3], box[i][4], box[i][1]);
		//Back face
		checkTriangle(&packet, box[i][4], box[i][6], box[i][7]);
		checkTriangle(&packet, box[i][4], box[i][5], box[i][6]);
		//Left face
		checkTriangle(&packet, box[i][0], box[i][5], box[i][4]);
		checkTriangle(&packet, box[i][0], box[i][1], box[i][5]);
		//Right face
		checkTriangle(&packet, box[i][7], box[i][2], box[i][3]);
		checkTriangle(&packet, box[i][7], box[i][6], box[i][2]);
		//Top face
		checkTriangle(&packet, box[i][2], box[i][5], box[i][1]);
		checkTriangle(&packet, box[i][2], box[i][6], box[i][5]);
		//Bottom face
		checkTriangle(&packet, box[i][7], box[i][0], box[i][4]);
		checkTriangle(&packet, box[i][7], box[i][3], box[i][0]);
		}
	}
	if (packet.foundCollision == true)
	{
		float nearDistance = 0.05f;
		vec3 elipsoid = packet.elipsoidRadius;
		vec3 intersection = packet.intersectionPoint;
		mat3 invCMD = mat3(
			vec3(elipsoid.x, 0.0f, 0.0f),
			vec3(0.0f, elipsoid.y, 0.0f),
			vec3(0.0f, 0.0f, elipsoid.z)
		);

		vec3 destination = packet.eBasePoint + packet.eVelocity;
		vec3 newBasePoint = packet.eBasePoint;

		//Set the length of the movement vector so we don't move close but not right to the collision point
		if (packet.nearestDistance >= nearDistance)
		{
			vec3 V = (packet.nearestDistance - nearDistance) * normalize(packet.eVelocity);
			newBasePoint = newBasePoint + V;

			//Move the intersection point closer to calculate the sliding plane
			packet.intersectionPoint -= nearDistance * normalize(V);
		}

		//Calculate a sliding plane
		vec3 slidePoint = packet.intersectionPoint;
		vec3 slideNormal = newBasePoint - packet.intersectionPoint;
		slideNormal = normalize(slideNormal);
		Plane slidingPlane(slidePoint, slideNormal);

		vec3 newDestination = destination - slidingPlane.signedDistanceTo(destination) * slideNormal;

		vec3 newVelocity = newDestination - packet.intersectionPoint;

		//Transform back to R3 and remove vertical component;
		result = invCMD * newVelocity;
		result.y = 0.0f;
		slidePoint = invCMD * slidePoint;
	}
	else
	{
		result = R3velocity;
		result.y = 0.0f;
	}
	
	return result;
}

//Assume that p1, p2 and p3 are given in front-facing order
void CollisionManager::checkTriangle(CollisionPacket* col, vec3 p1, vec3 p2, vec3 p3)
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
		else if (t0 < -30.0f || t1 > 30.0f)
		{
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
			edge = p3 - p2;
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