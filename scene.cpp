#include "scene.h"

scene::scene(const char* filename)
{
	parse(filename);
	cout<<endl<<"background "<<glm::to_string(bgColor);
	cout << endl << "ambient " << glm::to_string(ambLight);
	cout << endl << "eye " << glm::to_string(eye);
	cout << endl << "lookAt " << glm::to_string(lookAt);
	cout << endl << "up " << glm::to_string(up);
	cout << endl << "fovy " << fovy;
	cout<<endl;
}

glm::vec3 scene::rayTrace(glm::vec3 eye, glm::vec3 dir, int recurseDepth)
{
	//start with black, add color as we go
	glm::vec3 answer(0,0,0);

	//test for intersection against all our objects
	float dist = myObjGroup->testIntersections(eye, dir);
	//if we saw nothing, return the background color of our scene
	if (dist==9999999)
		return bgColor;
	

	//get the material index and normal vector(at the point we saw) of the object we saw
	int matIndex = myObjGroup->getClosest()->getMatIndex();
	glm::vec3 normal = myObjGroup->getClosest()->getNormal(eye, dir);

	//determine texture color
	glm::vec3 textureColor;
	if (myMaterials.at(matIndex).texture==NULL)
		//this is multiplicative, rather than additive
		//so if there is no texture, just use ones
		textureColor = glm::vec3(1,1,1);
	else
	{
		//if there is a texture image, ask the object for the image coordinates (between 0 and 1)
		glm::vec3 coords = myObjGroup->getClosest()->getTextureCoords(eye, dir);
		//get the color from that image location
		if (coords.x <= 1){
			textureColor = glm::vec3(
				PIC_PIXEL(myMaterials.at(matIndex).texture, (int)(myMaterials.at(matIndex).texture->nx*coords.x), (int)(myMaterials.at(matIndex).texture->ny*coords.y), 0),
				PIC_PIXEL(myMaterials.at(matIndex).texture, (int)(myMaterials.at(matIndex).texture->nx*coords.x), (int)(myMaterials.at(matIndex).texture->ny*coords.y), 1),
				PIC_PIXEL(myMaterials.at(matIndex).texture, (int)(myMaterials.at(matIndex).texture->nx*coords.x), (int)(myMaterials.at(matIndex).texture->ny*coords.y), 2));
			textureColor = textureColor*(1.0f / 255.0f);
		}
	}

	//add diffuse color times ambient light to our answer
	answer += multiplyColorVectors(myMaterials.at(matIndex).diffuseCol, ambLight);
	glm::vec3 point = eye + dist*dir;
	
	//iterate through all lights
	if (myLights.size() > 0) {
		vector<light>::iterator litItr = myLights.begin();
		glm::vec3 e = glm::normalize(eye - point);
		while (litItr != myLights.end()){
			light lt(*litItr); 
			glm::vec3 accumLight(1, 1, 1);
			//if the light can see the surface point,
			//add its diffuse color to a total diffuse for the point (using our illumination model)
			//use testIntersection to help decide this
			
			glm::vec3 l = glm::normalize(lt.position - point);
			float lDist = myObjGroup->testIntersections(point + l*0.01f, l);
			int i = 0;
			while (i < recurseDepth && lDist < 9999999){
				accumLight /= (1 + 0.02f*lDist + 0.08f*lDist*lDist);
				i++;
				lDist = myObjGroup->testIntersections(point + l*lDist, l);
			}	
			if (lDist == 9999999){
				glm::vec3 h = (e + l) / glm::length(e + l);
				glm::vec3 lambLight = multiplyColorVectors(myMaterials.at(matIndex).diffuseCol, lt.color) * glm::max(0.0f, glm::dot(normal, l));
				answer += multiplyColorVectors(lambLight, accumLight);
				glm::vec3 phongLight = multiplyColorVectors(myMaterials.at(matIndex).specularCol, lt.color) * powf(glm::dot(h, normal), myMaterials.at(matIndex).shininess);
				phongLight = multiplyColorVectors(phongLight, accumLight);
				answer += phongLight;
				//add the diffuse light times the accumulated diffuse light to our answer
			}
			litItr++;
		}
	}
	
	if (recurseDepth<3)
	{
		//reflect our view across the normal
		//recusively raytrace from the surface point along the reflected view
		//add the color seen times the reflective color
		//reflection
		glm::vec3 reflect = (normal * glm::dot(dir, normal) * -2.0f) + dir;
		reflect = glm::normalize(reflect);
		glm::vec3 color = rayTrace(point+reflect*0.01f, reflect, recurseDepth + 1);
		if (color != bgColor) {
			color = multiplyColorVectors(color, myMaterials.at(matIndex).reflectiveCol);
			answer += color;
		}
		
		//if going into material (dot prod of dir and normal is negative), bend toward normal
		//find entry angle using inverse cos of dot product of dir and -normal
		//multiply entry angle by index of refraction to get exit angle
		//else, bend away
		//find entry angle using inverse cos of dot product of dir and normal
		//divide entry angle by index of refraction to get exit angle
		//recursively raytrace from the other side of the object along the new direction
		//add the color seen times the transparent color
		// refraction  (partially followed comments above and found equation in book chapter 13.1 page 305)
		float refraction = myMaterials.at(matIndex).refractionIndex;
		glm::vec3 n = normal;
		float in = glm::dot(dir, normal);
		if (in > 0){
			refraction = 1 / refraction;
			n = -n;
		}
		float discrim = 1 - (powf(refraction, 2) * (1 - powf(glm::dot(dir,n),2)));
		if (discrim > 0){
			glm::vec3 refract = (refraction*(dir - n*glm::dot(dir, n))) - (n * sqrtf(discrim));
			refract = glm::normalize(refract);
			color = rayTrace(point + refract*0.01f, refract, recurseDepth + 1);
		}
		
		answer += multiplyColorVectors(color, myMaterials.at(matIndex).transparentCol);
	}

	//multiply whatever color we have found by the texture color
	answer=multiplyColorVectors(answer,textureColor);
	return answer;
}

glm::vec3 scene::multiplyColorVectors(glm::vec3 a, glm::vec3 b) //multiplies the terms of two color-holding glm::vec3 vars
{
	glm::vec3 result(a.x * b.x, a.y * b.y, a.z * b.z);
	return result;
}

glm::vec3 scene::getEye()
{
	return eye;
}

glm::vec3 scene::getLookAt()
{
	return lookAt;
}

glm::vec3 scene::getUp()
{
	return up;
}

float scene::getFovy()
{
	return fovy;
}

void scene::parse(const char *filename)
{
	//some default values in case parsing fails
	myObjGroup = NULL;
	bgColor = glm::vec3(0.5,0.5,0.5);
	ambLight = glm::vec3(0.5,0.5,0.5);
	eye = glm::vec3(0,0,0);
	lookAt = glm::vec3(0,0,-1);
	up = glm::vec3(0,1,0);
	fovy = 45;
	file = NULL;
	curline = 1;

	//the file-extension needs to be "ray"
	assert(filename != NULL);
	const char *ext = &filename[strlen(filename)-4];
	assert(!strcmp(ext,".ray"));
	fopen_s(&file, filename, "r");
	assert(file != NULL);

	char token[MAX_PARSER_TOKEN_LENGTH];

	//prime the parser pipe
	parse_char = fgetc(file);

	while (getToken(token))
	{
		if(!strcmp(token, "Background"))
			parseBackground();
		else if(!strcmp(token, "Camera"))
			parseCamera();
		else if(!strcmp(token, "Materials"))
			parseMaterials();
		else if(!strcmp(token, "Group"))
			myObjGroup = parseGroup();
		else if(!strcmp(token, "Lights"))
			parseLights();
		else
		{
			cout<<"Unknown token in parseFile: '"<<token<<
				"' at input line "<<curline<<"\n";
			exit(-1);
		}
	}
}

/* Parse the "Camera" token */
void scene::parseCamera()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	float x,y,z;
	getToken(token); assert (!strcmp(token, "{"));

	//get the eye, center, and up vectors (similar to gluLookAt)
	getToken(token); assert (!strcmp(token, "eye"));
	x = readFloat();
	y = readFloat();
	z = readFloat();
	eye = glm::vec3(x,y,z);

	getToken(token); assert (!strcmp(token, "lookAt"));
	x = readFloat();
	y = readFloat();
	z = readFloat();
	lookAt = glm::vec3(x, y, z);

	getToken(token); assert (!strcmp(token, "up"));
	x = readFloat();
	y = readFloat();
	z = readFloat();
	up = glm::vec3(x, y, z);

	getToken(token); assert (!strcmp(token, "fovy"));
	fovy = readFloat();

	getToken(token); assert (!strcmp(token, "}"));
}

/* Parses the "Background" token */
void scene::parseBackground()
{
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token); assert (!strcmp(token, "{"));
	while (1)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			break;
		}
		else if (!strcmp(token, "color"))
		{
			bgColor = readVec3();
		}
		else if (!strcmp(token, "ambientLight"))
		{
			ambLight = readVec3();
		}
		else
		{
			cout<< "Unknown token in parseBackground: "<<token<<"\n";
			assert(0);
		}
	}
}

/* Parses the "Group" token */
rtObjGroup* scene::parseGroup()
{
	char token[MAX_PARSER_TOKEN_LENGTH];


	getToken(token);
	assert (!strcmp(token, "{"));

	/**********************************************/
	/* Instantiate the group object               */
	/**********************************************/
	rtObjGroup *answer     = new rtObjGroup();

	bool working=true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token,"}"))
		{
			working=false;
		}
		else
		{
			if (!strcmp(token, "Sphere"))
			{
				sphere *sceneElem = parseSphere();
				assert(sceneElem != NULL);
				answer->addObj(sceneElem);
			}
			else if (!strcmp(token, "Triangle"))
			{
				triangle *sceneElem = parseTriangle();
				assert(sceneElem != NULL);
				answer->addObj(sceneElem);
			}
			else
			{
				cout << "Unknown token in Group: '" << token << "' at line "
					<< curline << "\n";
				exit(0);
			}
		}
	}

	/* Return the group */
	return answer;
}

/* Parse the "Sphere" token */
sphere* scene::parseSphere()
{
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token); assert (!strcmp(token, "{"));
	getToken(token); assert (!strcmp(token, "materialIndex"));
	int sphere_material_index = readInt();
	getToken(token); assert (!strcmp(token, "center"));
	glm::vec3 center = readVec3();
	getToken(token); assert (!strcmp(token, "radius"));
	float radius = readFloat();
	getToken(token); assert (!strcmp(token, "}"));

	/**********************************************/
	/* The call to your own constructor goes here */
	/**********************************************/
	return new sphere(center,radius,sphere_material_index,this);
}

/* Parse out the "Triangle" token */
triangle* scene::parseTriangle()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert (!strcmp(token, "{"));

	/* Parse out vertex information */
	getToken(token); assert (!strcmp(token, "vertex0"));
	glm::vec3 v0 = readVec3();
	getToken(token); assert (!strcmp(token, "vertex1"));
	glm::vec3 v1 = readVec3();
	getToken(token); assert (!strcmp(token, "vertex2"));
	glm::vec3 v2 = readVec3();
	getToken(token); assert (!strcmp(token, "tex_xy_0"));
	float x0 = 0;
	float y0 = 0;
	x0 = readFloat();
	y0 = readFloat();
	getToken(token); assert (!strcmp(token, "tex_xy_1"));
	float x1 = 0;
	float y1 = 0;
	x1 = readFloat();
	y1 = readFloat();
	getToken(token); assert (!strcmp(token, "tex_xy_2"));
	float x2 = 0;
	float y2 = 0;
	x2 = readFloat();
	y2 = readFloat();
	getToken(token); assert (!strcmp(token, "materialIndex"));
	int mat = readInt();

	getToken(token); assert (!strcmp(token, "}"));

	/**********************************************/
	/* The call to your own constructor goes here */
	/**********************************************/
	return new triangle(v0,v1,v2,x0,x1,x2,y0,y1,y2,mat,this);
}

/* Parse the "Materials" token */
void scene::parseMaterials()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	char texname[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert (!strcmp(token, "{"));

	/* Loop over each Material */
	bool working=true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			working = false;
		}
		else if (!strcmp(token, "Material"))
		{
			getToken(token); assert (!strcmp(token, "{"));
			texname[0] = '\0';
			glm::vec3 diffuseColor(1,1,1);
			glm::vec3 specularColor(0,0,0);
			float shininess = 1;
			glm::vec3 transparentColor(0,0,0);
			glm::vec3 reflectiveColor(0,0,0);
			float indexOfRefraction = 1;

			while (1)
			{
				getToken(token);
				if (!strcmp(token, "textureFilename"))
				{
					getToken(token);
					strcpy(texname, token);
				}
				else if (!strcmp(token, "diffuseColor"))
					diffuseColor = readVec3();
				else if (!strcmp(token, "specularColor"))
					specularColor = readVec3();
				else if  (!strcmp(token, "shininess"))
					shininess = readFloat();
				else if (!strcmp(token, "transparentColor"))
					transparentColor = readVec3();
				else if (!strcmp(token, "reflectiveColor"))
					reflectiveColor = readVec3();
				else if (!strcmp(token, "indexOfRefraction"))
					indexOfRefraction = readFloat();
				else
				{
					assert (!strcmp(token, "}"));
					break;
				}
			}

			material temp;
			temp.diffuseCol=diffuseColor;
			temp.specularCol=specularColor;
			temp.shininess=shininess;
			temp.transparentCol=transparentColor;
			temp.reflectiveCol=reflectiveColor;
			temp.refractionIndex=indexOfRefraction;
			if(strcmp(texname, "NULL"))
				temp.texture=jpeg_read(texname, NULL);
			else
				temp.texture=NULL;

			myMaterials.push_back(temp);

		}
	}
}

void scene::parseLights()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	char texname[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert (!strcmp(token, "{"));

	/* Loop over each Material */
	bool working=true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			working = false;
		}
		else if (!strcmp(token, "Light"))
		{
			getToken(token); assert (!strcmp(token, "{"));
			texname[0] = '\0';
			glm::vec3 position(0,0,0);
			glm::vec3 color(1,1,1);

			while (1)
			{
				getToken(token);
				if (!strcmp(token, "position"))
					position = readVec3();
				else if (!strcmp(token, "color"))
					color = readVec3();
				else
				{
					assert (!strcmp(token, "}"));
					break;
				}
			}

			/**********************************************/
			/* The call to your own constructor goes here */
			/**********************************************/
			light temp;
			temp.position=position;
			temp.color=color;

			myLights.push_back(temp);

		}
	}
}

/* consume whitespace */
void scene::eatWhitespace(void)
{
	bool working = true;

	do{
		while(isspace(parse_char))
		{
			if(parse_char == '\n')
			{
				curline++;
			}
			parse_char = fgetc(file);
		}

		if('#'==parse_char)
		{
			/* this is a comment... eat until end of line */
			while(parse_char != '\n')
			{
				parse_char = fgetc(file);
			}

			curline++;
		}
		else
		{
			working = false;
		}

	} while(working);
}

/* Parse out a single token */
int scene::getToken(char token[MAX_PARSER_TOKEN_LENGTH])
{
	int idx=0;

	assert (file != NULL);
	eatWhitespace();

	if(parse_char == EOF)
	{
		token[0]='\0';
		return 0;
	}
	while((!(isspace(parse_char))) && (parse_char != EOF))
	{
		token[idx]=parse_char;
		idx++;
		parse_char = fgetc(file);
	}

	token[idx] = '\0';
	return 1;
}

/* Reads in a 3-vector */
glm::vec3 scene::readVec3()
{
	float x,y,z;

	x=readFloat();
	y=readFloat();
	z=readFloat();

	return glm::vec3(x,y,z);
}

/* Reads in a single float */
float scene::readFloat()
{
	float answer;
	char buf[MAX_PARSER_TOKEN_LENGTH];

	if(!getToken(buf))
	{
		cout<< "Error trying to read 1 float (EOF?)\n";
		assert (0);
	}

	int count = sscanf(buf,"%f",&answer);
	if (count != 1)
	{
		cout<< "Error trying to read 1 float\n";
		assert (0);

	}
	return answer;
}

/* Reads in a single int */
int scene::readInt()
{
	int answer;
	char buf[MAX_PARSER_TOKEN_LENGTH];

	if(!getToken(buf))
	{
		cout<< "Error trying to read 1 int (EOF?)\n";
		assert (0);
	}

	int count = sscanf(buf,"%d",&answer);
	if (count != 1)
	{
		cout<< "Error trying to read 1 int\n";
		assert (0);

	}
	return answer;
}