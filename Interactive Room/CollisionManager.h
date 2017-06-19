#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H
#include "glm.hpp"
#include "collision_math.h"
#include <vector>
#include <stdio.h>
#include <string>
#include <iostream>

//Collision manager, algorithm and plane classes referenced from Soren Seeberg
//http://www.peroxide.dk/papers/collision/collision.pdf

class Model;

class CollisionManager
{
public:
	
	//prototype for static accessor
	static CollisionManager *getInstance();
	void trackModel(Model* model);
	vec3 askMove(glm::vec3 elipsoidradius, glm::vec3 R3velocity, glm::vec3 R3position);
	bool getLowestRoot(float a, float b, float c, float current, float* root);
	bool checkPointInTriangle(const vec3 &point, const vec3 &p1, const vec3 &p2, const vec3 & p3);

private:
	std::vector<CollisionPacket*> move_queue;
	std::vector<Model*> tracked_models;
	void checkTriangle(CollisionPacket* col, vec3 p1, vec3 p2, vec3 p3);
};
#endif