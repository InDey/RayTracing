The starter code reads a scene file from the file specified as a command line argument.

Most of the code you will need to write is in the object called "scene".

Look at the private data members of scene (under the words "VERY USEFUL"), here you will find the necessary information for raytracing.

Your primary task is to add to the raytrace function in scene, to make it return a Vec3f containing the appropriate RGB values at the location.

You will also need to define the intersection testing function for the two types of rtObjects (raytracer objects): triangles and spheres.

Use the lectures / book to figure out intersection testing. Use the phong illumination model and follow the comments in the raytrace function to figure out shading. Good luck!