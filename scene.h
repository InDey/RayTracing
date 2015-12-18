#ifndef SCENE
#define SCENE

#include <pic.h>
//#include "util_vectors.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "rtObjGroup.h"
#include "light.h"
#include "rtObject.h"
#include "sphere.h"
#include "triangle.h"
#include "material.h"

class sphere;
class triangle;

// For parsing purposes
#define MAX_PARSER_TOKEN_LENGTH 100

class scene
{
public:
	//constructor from a scene text file
	scene(const char* filename);

	//accessors for the camera properties
	glm::vec3 getEye();
	glm::vec3 getLookAt();
	glm::vec3 getUp();
	float getFovy();

	//returns the rgb color seen from this eye in this direction
	glm::vec3 rayTrace(glm::vec3 eye, glm::vec3 dir, int recurseDepth);

	//destructor
	~scene();

private:
	//some functions and variables used in parsing
	void parse(const char* filename);
	void eatWhitespace(void);
    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
    glm::vec3 readVec3();
    float readFloat();
    int readInt();
	void parseBackground();
	void parseCamera();
	rtObjGroup* parseGroup();
	sphere* parseSphere();
	void parseMaterials();
	void parseMaterialIndex();
	triangle* parseTriangle();
	void parseLights();
	FILE *file;
	int parse_char; //current character read from file
    int curline;    //current line in the file

	//!!!VERY USEFUL!!!
	//performs an element-wise multiplication of two glm::vec3s
	glm::vec3 multiplyColorVectors(glm::vec3 a, glm::vec3 b);

	//the group of objects that are part of the scene
	//you may want to utilize multiple groups for view volume segmentation
	rtObjGroup* myObjGroup;
	vector<material> myMaterials;
	vector<light> myLights;

	glm::vec3 bgColor; //background color
    glm::vec3 ambLight; //ambient light
    glm::vec3 eye; //camera location
    glm::vec3 lookAt; //camera looking at (x, y, z)
    glm::vec3 up; //camera up vector
    float fovy; //camera vertical angle of view

};

#endif