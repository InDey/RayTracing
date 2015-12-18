#ifndef LIGHT
#define LIGHT

#include "util_vectors.h"

//struct that holds a 3D location and an RGB color
struct light
{
	glm::vec3 position, color;
};

#endif