#include <fstream>
#include <vector>

#include <glm/glm.hpp>
#include "helper.h"

using namespace std;

// simple structure to hold three integer-values (used for triangle indicies)
struct tri {
  int A;
  int B;
  int C;
};


// Declaration:

class OffObject 
{

public: 
	
  	vector<glm::vec3> vertexList;
	vector<glm::vec3> normalsList;
	vector<tri> faceList;
	
	int noOfFaces;
	int noOfVertices;
	
	OffObject(string filename);
	
};



//TODO: IMPLEMENTATION:

// the constuctor recieves the filename (an .off file) and parses it. The vertices, normals and triangles
// are pushed back into the respective container whereas the NORMALS have to be explicitly computed for each
// vertex. 

OffObject::OffObject(string filename) {
 
	std::ifstream inFile(filename.c_str());
	char tmp[20];

	inFile >> tmp;
	inFile >> noOfVertices;
	inFile >> noOfFaces;
	inFile >> tmp;

	// Read Vertex Data and initialize the normals:	
    for (int i=0; i<noOfVertices; i++) 
	{
		glm::vec3 vertex;
        // TODO

        // initalize the normal with (0,0,0)
		
		// add vertex and normal
    }


	// Read Triangle Data:
	tri T;
    for (int i=0; i<noOfFaces; i++) 
	{
        // TODO
        // probably helpful: glm::cross(..,..);	//CHECK DOCUMENTATION!!!
    }
    
    //normalize:
    for (int i=0; i<noOfVertices; i++) 
	{
        // TODO
        // probably helpful: glm::normalize(..);	//CHECK DOCUMENTATION!!!
    }      


}

