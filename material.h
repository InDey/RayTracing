#ifndef MATERIAL
#define MATERIAL

#include <pic.h>
//#include "util_vectors.h"
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

struct material
{
	//the image loaded using libpicio
	Pic* texture;
	//four RGB colors for different material properties
	glm::vec3 diffuseCol, specularCol, transparentCol, reflectiveCol;

	//the shininess (see illumination model)
	//and refraction index (look up refraction of light through materials)
	float shininess, refractionIndex;
};

#endif