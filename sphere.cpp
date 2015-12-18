#include "sphere.h"

//constructor given  center, radius, and material
sphere::sphere(glm::vec3 c, float r, int m, scene* s) : rtObject(s)  
{
	center = c;
	radius = r;
	matIndex = m;
	myScene = s;
}

float sphere::testIntersection(glm::vec3 eye, glm::vec3 dir)
{
	//see the book for a description of how to use the quadratic rule to solve
	//for the intersection(s) of a line and a sphere, implement it here and
	//return the minimum positive distance or 9999999 if none
	dir = glm::normalize(dir);
	float dd = glm::dot(dir, dir);
	float d_ec = glm::dot(dir, eye - center);
	float ec_ec = glm::dot(eye - center, eye - center);
	float determ = sqrtf(d_ec*d_ec - dd * (ec_ec - radius * radius));
	if (determ < 0)
		return 9999999;
	float t1 = (-d_ec + determ)/dd;
	float t2 = (-d_ec - determ)/dd;
	if (t1 < 0 || t2 < 0 || t1 > 9999999 || t2 > 9999999){
		return 9999999;
	}
	float t = glm::min(t1, t2);
	glm::vec3 p = eye + t*dir;
	glm::vec3 dist = p - eye;
	if (glm::length(dist) > 9999999)
		return 9999999;
	return glm::length(dist);
}

glm::vec3 sphere::getNormal(glm::vec3 eye, glm::vec3 dir)
{
	//once you can test for intersection,
	//simply add (distance * view direction) to the eye location to get surface location,
	//then subtract the center location of the sphere to get the normal out from the sphere
	glm::vec3 p = eye + testIntersection(eye, dir) * dir;
	glm::vec3 normal = p - center;
	return normalize(normal);
}

glm::vec3 sphere::getTextureCoords(glm::vec3 eye, glm::vec3 dir)
{
	//find the normal as in getNormal

	//use these to find spherical coordinates
	glm::vec3 x(1,0,0);
	glm::vec3 z(0,0,1);
	glm::vec3 p = (eye + testIntersection(eye, dir) * dir) - center;
	float O = acos(p.z/ radius);
	float P = atan2f(p.y, p.x);

	float u = P / (2 * 3.1416);
	float v = 1 - (3.1416 - O) / 3.1416;

	//phi is the angle down from z
	//theta is the angle from x curving toward y
	//hint: angle between two vectors is the acos() of the dot product

	//find phi using normal and z

	//find the x-y projection of the normal

	//find theta using the x-y projection and x

	//if x-y projection is in quadrant 3 or 4, then theta=2*PI-theta
	
	//return coordinates scaled to be between 0 and 1
	glm::vec3 coords(u, v, 0);
	return coords;
}