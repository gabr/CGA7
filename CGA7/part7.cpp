/// This skeleton only contains a lit sphere. We recommend to use your stuff from the
/// the previous assignmnet as skeleton.

#include "camera.h"
#include "offLoader.h"

#include <QtOpenGL/QGLWidget>
#include <QtGui/qimage.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <sstream>

using namespace std;

// global consts
const double PI = 3.14159265359;

// some global variables:
bool wireframe_mode = false;

// window size
int window_widht = 1024;
int window_height = 1024;

// global camera variable
cameraSystem cam;
const float forwardDelta = 2;
const float angleDelta = 2.0f;
glm::vec2 mouseStartPosition;

// space
float spaceLength = 400;

float t = 0;  // the time parameter (incremented in the idle-function)
float speed = 0.01;  // rotation speed of the light source in degree/frame

// textures
glm::vec4 planetColor(1.0);
glm::vec4 backgroundColor(0.0);
GLubyte blackMask[] = { 0 };
GLuint earthTex, earthMaskTex, moonTex, saturnTex, backgroundTex, blackMaskTex; // use for the according textures

// file paths
const string background_filePath = "./data/background.jpg";
const string earth_filePath = "./data/earth.jpg";
const string earthMask_filePath = "./data/earth_reflect.jpg";
const string moon_filePath = "./data/moon.jpg";
const string saturn_filePath = "./data/saturn.jpg";

const float planetSlices = 124;
const float planetStacks = 124;

// sun
const double sunRadius = 25;
const double sunSlices = 100;
const double sunStacks = 100;
glm::vec4 sunColor(1.0, 1.0, 0.0, 0.0);
glm::vec4 lightSource(0.0, 0.0, 0.0, 1.0);

// earth
const double earthRadius = 12;
const float earthDegree = 2.0;
glm::vec4 earthColor = planetColor;
// moon
const double moonRadius = 6;
const double moonSlices = 15;
const double moonStacks = 15;
const float moonDegree = 4.0;
glm::vec4 moonColor = planetColor;

// saturn
const double saturnRadius = 16;
const float saturnDegree = 1.0;
glm::vec4 saturnColor = planetColor;
// rings
const double distanceFromSaturn = 7.0;
const double distanceBetweenRings = 0.4;
const double numberOfRings = 10;
const double numberOfCircleSegments = 60;
glm::vec4 ringsColor(0.8, 0.6, 0.5, 0.0);

//We need to keep track of matrices ourselves
/**
* @brief P,V,M:
* your matrices for setting the scene, no matrix stack anymore
*/
glm::mat4 P, V, M;

/**
 * @brief The ShaderUniforms struct:
 * every shader has its own uniforms,
 * binding of uniforms is done in bindUniforms()
 *
 */
struct ShaderUniforms
{
	GLuint Shader;
	GLint location_MVP;
	GLint location_MV;
	GLint location_NormalMatrix ; 
	GLint location_Time;
	GLint location_LightSourceViewSpace;
    GLint location_Color;
    GLint location_Texture;
    GLint location_Mask;

    void bindUniforms(glm::mat4& M, glm::mat4& V, glm::mat4& P, glm::vec4& LightSource, glm::vec4& Color, GLuint TexdID, GLuint TexMaskID, float  t)
    {
		location_Time					= glGetUniformLocation(Shader, "Time");
		location_MVP					= glGetUniformLocation(Shader, "MVP");
		location_MV						= glGetUniformLocation(Shader, "MV");
		location_NormalMatrix			= glGetUniformLocation(Shader, "NormalMatrix");
		location_LightSourceViewSpace	= glGetUniformLocation(Shader, "LightSource");
        location_Color                  = glGetUniformLocation(Shader, "Color");

        location_Texture                = glGetUniformLocation(Shader, "Texture");
        location_Mask                   = glGetUniformLocation(Shader, "Mask");
 
		glm::mat4 MV			= V*M;
		glm::mat4 MVP			= P*MV;
		glm::mat3 NormalMatrix	= glm::transpose(glm::inverse(glm::mat3(MV)));


		glUniformMatrix4fv(location_MVP, 1, false, glm::value_ptr(MVP));
		glUniformMatrix4fv(location_MV, 1, false, glm::value_ptr(MV));
		glUniformMatrix3fv(location_NormalMatrix, 1, false, glm::value_ptr(NormalMatrix));
		glUniform4fv(location_LightSourceViewSpace, 1, glm::value_ptr(LightSource));
        glUniform4fv(location_Color, 1, glm::value_ptr(Color));
        glUniform1f(location_Time, 10 * t);

        glUniform1i(location_Texture, 0);
        glUniform1i(location_Mask, 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TexdID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TexMaskID);
	}

};

/**
 * @brief The ShaderUniforms struct:
 * every shader has its own uniforms,
 * binding of uniforms is done in bindUniforms()
 *
 */
ShaderUniforms SunShader, TexturePhongShader; // the shaders

/**
 * @brief OffObject
 *  Object for loading/parsing the shuttle file
 *
 */
OffObject *objA;    // the shuttle
 

/**
 * @brief The GeometryData struct:
 * store the VertexArrayObject and number of vertices and indices
 */
struct GeometryData
{
	GLuint vao;
	unsigned int numVertices;
	unsigned int numIndices; 
};

GeometryData geometryShuttle, geometryCube, geometrySphere, geometryRings;

/**
 * @brief The Vertex struct:
 * store vertices with according normals
 * and texture coordinates
 */
struct Vertex {

	Vertex(glm::vec3 p, glm::vec3 n )
	{
		position[0] = p.x;
		position[1] = p.y;
		position[2] = p.z;
		position[3] = 1;

		normal[0] = n.x;
		normal[1] = n.y;
		normal[2] = n.z;			 
		normal[3] = 1;
			 
	}; 
	Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t)	
	{
		position[0] = p.x;
		position[1] = p.y;
		position[2] = p.z;
		position[3] = 1;

		normal[0] = n.x;
		normal[1] = n.y;
		normal[2] = n.z;			 
		normal[3] = 1;

		texcoord[0] = t.x;
		texcoord[1] = t.y;
	};

	GLfloat position[4];
	GLfloat normal[4];
	GLfloat texcoord[2];

    std::string toString()
    {
        stringstream ss;
        ss << "[" << position[0] << "; " << position[1] << "; " << position[2] << "]";
        return ss.str();
    }

    std::string toStringTex()
    {
        stringstream ss;
        ss << "(" << texcoord[0] << ", " << texcoord[1] << ")";
        return ss.str();
    }
};

void bindVertexArrayObjects(GeometryData& geometry, const std::vector<Vertex> &vertexdata, const std::vector<unsigned short> &indices) 
{
	
	//*bind to GL
	glGenVertexArrays(1, &(geometry.vao));
    glBindVertexArray(geometry.vao);
	
	geometry.numVertices = vertexdata.size();
	geometry.numIndices = indices.size();

	// Create and bind a BO for vertex data
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, geometry.numVertices * sizeof(Vertex), vertexdata.data(), GL_STATIC_DRAW);
	

	// set up vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

	// Create and bind a BO for index data
	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// copy data into the buffer object
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.numIndices * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);


    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

 void createRings() 
{ 
	glm::vec2 t;
    glm::vec3 n(0,1,0);    
    glm::vec3 p ;
	
	unsigned short index = 0;
	std::vector<Vertex> vertexdata ;
	std::vector<unsigned short> indices;
	

	 
	float r= 1;
	for (int i=0;i<100;i++)
	{
		float arg = PI*(float)i/50.0;
		p =  glm::vec3(r*cos(arg),0,r*sin(arg));	 
		vertexdata.push_back(Vertex (p,n,t));	
		indices.push_back(index++);  
	}
	 
	bindVertexArrayObjects(geometryRings,vertexdata,indices);
		 
}


/// TODO
 void createSphere()
{
	float r=1;
	
	float dTheta = 2.0*PI/planetStacks;
	float dPhi = PI/planetSlices;

    float x, y, z;

    glm::vec2 t;    // texture
    glm::vec3 p, n; // position and normal

	std::vector<Vertex> vertexdata;
	std::vector<unsigned short> indices;

    // top
    n = p = glm::vec3(0, r, 0);
    t = glm::vec2(0.5f, 0.0f);
    vertexdata.push_back(Vertex(p, n, t));

    // middle
    for (int j = 1; j < planetSlices; j++)
    {
        for (int i = planetStacks; i >= 0; i--)
        {
            x = sin(j*dPhi)*cos(i*dTheta);
            y = cos(j*dPhi);
            z = sin(j*dPhi)*sin(i*dTheta);

            t = glm::vec2(((float)(i)) / ((float)(planetStacks)), ((float)j) / planetSlices);
            n = p = glm::vec3(r*x, r*y, r*z);
            vertexdata.push_back(Vertex(p, n, t));
        }
    }

    // bottom
    n = p = glm::vec3(0, -r, 0);
    t = glm::vec2(0.5f, 1.0f);
    vertexdata.push_back(Vertex(p, n, t));

    // indices
    // top and bottom
    int vCount = vertexdata.size() - 1;
    for (int i = 1; i <= planetStacks; i++)
    {
        // top
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);

        // bottom
        indices.push_back(vCount);
        indices.push_back(vCount - i);
        indices.push_back(vCount - i - 1);
    }
    

    // middle
    for (int i = planetStacks + 2; i < vCount ; i++)
    {
        if (i % ((int)planetStacks + 1) == 0) // dont create triangles for additional vertexes
            continue;

            indices.push_back(i);
            indices.push_back(i - planetStacks);
            indices.push_back(i - planetStacks - 1);

            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i - planetStacks );
    }

	bindVertexArrayObjects(geometrySphere,vertexdata,indices);
}

/// creates a cube with length a
 void createCube(float a)
{	
	std::vector<Vertex> vertexdata;
	std::vector<unsigned short> indices;

	glm::vec2 t;
	glm::vec3 n;
	glm::vec3 p;
	 
	unsigned short index = 0;
	t = glm::vec2(0,0);
	p = glm::vec3(a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(0,1);
	p =glm::vec3(-a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,1);
	p =glm::vec3(-a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,0);
	p =glm::vec3(a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);

	// +Z
    
	t =glm::vec2(0,0);
	p =glm::vec3(a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(0,1);
	p =glm::vec3(a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,1);
	p =glm::vec3(-a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,0);
	p =glm::vec3(-a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
      
      
	// +X     
	t =glm::vec2(0,0);
	p =glm::vec3(a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(0,1);
	p =glm::vec3(a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,1);
	p =glm::vec3(a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,0);
	p =glm::vec3(a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
           
	// -X      
	t =glm::vec2(0,0);
	p =glm::vec3(-a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(0,1);
	p =glm::vec3(-a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,1);
	p =glm::vec3(-a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,0);
	p =glm::vec3(-a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
      
	// +Y
	t =glm::vec2(0,0);
	p =glm::vec3(-a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(0,1);
	p =glm::vec3(a,a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,1);
	p =glm::vec3(a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t =glm::vec2(1,0);
	p =glm::vec3(-a,a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
      
	// +Y
	t = glm::vec2(0,0);
	p = glm::vec3(-a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t = glm::vec2(0,1);
	p = glm::vec3(-a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t = glm::vec2(1,1);
	p = glm::vec3(a,-a,a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
	t = glm::vec2(1,0);
	p = glm::vec3(a,-a,-a);
	vertexdata.push_back(Vertex(p,n,t));
	indices.push_back(index++);
      

	 bindVertexArrayObjects(geometryCube,vertexdata,indices);
}
  
/// TODO
/// loads shuttle data from off-file and fills according GeometryData
 void createShuttle()
{
    objA = new OffObject("../data/shuttle.off");
   
   
    std::vector<Vertex> vertexdata ;        // store the vertices of the shuttle here
    std::vector<unsigned short> indices;    // store the according indices here


	geometryShuttle.numVertices = objA->noOfVertices;
	geometryShuttle.numIndices = objA->noOfFaces*3;


	// TODO: Fill vertexdata
	
	for (int i=0; i< geometryShuttle.numVertices; i++) 
	{	 
		 		  
	}

	// TODO: Fill indexData

	for (int i=0; i< objA->noOfFaces; i++) 
	{ 
	
	}

	bindVertexArrayObjects(geometryShuttle,vertexdata,indices);

}

void initTexture(GLint name, GLint w, GLint h, GLubyte *data) {
	  
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w,h,0,GL_RGBA,GL_UNSIGNED_BYTE, data);
}



void initGL() {
  
	glClearColor(0,0,0,0);   // set the clear color to black
	glEnable(GL_DEPTH_TEST); // turn on the depth test
	glEnable(GL_CULL_FACE);  // turn on backface culling
   
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   
  
	// set the camera:
    V = cam.getView();
	
    // create the shaders (the functions are defined in helper.h)
      createProgram_VF("sun_VS.glsl","sun_FS.glsl",&SunShader.Shader);
     createProgram_VF("Light_and_Tex_VS.glsl","Light_and_Tex_FS.glsl",&TexturePhongShader.Shader);

  
// ***********************************************************************************
// You can add texture creation from previous assignment

     glGenTextures(1, &backgroundTex);
     QImage background = QGLWidget::convertToGLFormat(QImage(background_filePath.c_str()));
     initTexture(backgroundTex, background.width(), background.height(), background.bits());

     glGenTextures(1, &earthTex);
     QImage img(earth_filePath.c_str());
     QTransform transform;
     transform.rotate(180);
     img = img.transformed(transform);
     QImage earth = QGLWidget::convertToGLFormat(img);
     initTexture(earthTex, earth.width(), earth.height(), earth.bits());

     glGenTextures(1, &earthMaskTex);
     QImage imgMask(earthMask_filePath.c_str());
     imgMask = imgMask.transformed(transform);
     QImage earthMask = QGLWidget::convertToGLFormat(imgMask);
     initTexture(earthMaskTex, earthMask.width(), earthMask.height(), earthMask.bits());

     glGenTextures(1, &moonTex);
     QImage moon = QGLWidget::convertToGLFormat(QImage(moon_filePath.c_str()));
     initTexture(moonTex, moon.width(), moon.height(), moon.bits());

     glGenTextures(1, &saturnTex);
     QImage saturn = QGLWidget::convertToGLFormat(QImage(saturn_filePath.c_str()));
     initTexture(saturnTex, saturn.width(), saturn.height(), saturn.bits());

     glGenTextures(1, &blackMaskTex);
     initTexture(blackMaskTex, 1, 1, blackMask);
// ***********************************************************************************
   
   
   // the space shuttle & other geometry:
   createShuttle();
   createSphere();
   createRings();
   createCube(spaceLength);

   printf("Init done!");
}

void reshape(int w, int h)
{
	glViewport(0,0,(GLsizei) w, (GLsizei) h);
 
  	P = glm::perspective(70.0f, (GLfloat) w/ (GLfloat) h, 10.0f, 1500.0f);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// create Matrices
	//M = glm::rotate(50.0f,0.0f,1.0f,0.0f);  

	glm::vec4 Color (0.9,0.9,0, 1); // set the color to yellow

	// bind Shader & bind Uniforms
    glUseProgram(SunShader.Shader);
    M = glm::mat4(1.0f) * glm::rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    SunShader.bindUniforms(M, V, P, lightSource, sunColor, 0, 0, t);
    glutSolidSphere(sunRadius, sunSlices, sunStacks);
	

    glUseProgram(TexturePhongShader.Shader);

  
	// ***********************************************************************************
    // background
    M = glm::mat4(1.0f);
    TexturePhongShader.bindUniforms(M, V, P, lightSource, backgroundColor, backgroundTex, blackMaskTex, t);
    glBindVertexArray(geometryCube.vao);
    glDrawElements(GL_QUADS, geometryCube.numIndices, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);

    // earth
    M = glm::rotate(earthDegree * t, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::translate(glm::vec3(50.0f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(earthRadius));
    TexturePhongShader.bindUniforms(M, V, P, lightSource, planetColor, earthTex, earthMaskTex, t);
    glBindVertexArray(geometrySphere.vao);
    glDrawElements(GL_TRIANGLES, geometrySphere.numIndices, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);

    // moon
    M = glm::rotate(earthDegree * t, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::translate(glm::vec3(50.0f, 0.0f, 0.0f))
        * glm::rotate(moonDegree * t, glm::vec3(0.0f, 0.0f, 1.0f))
        * glm::translate(glm::vec3(20.0f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(moonRadius));
    TexturePhongShader.bindUniforms(M, V, P, lightSource, planetColor, moonTex, blackMaskTex, t);
    glBindVertexArray(geometrySphere.vao);
    glDrawElements(GL_TRIANGLES, geometrySphere.numIndices, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);

    // saturn
    M = glm::rotate(saturnDegree * t, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::translate(glm::vec3(100.0f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(saturnRadius));
    TexturePhongShader.bindUniforms(M, V, P, lightSource, planetColor, saturnTex, blackMaskTex, t);
    glBindVertexArray(geometrySphere.vao);
    glDrawElements(GL_TRIANGLES, geometrySphere.numIndices, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);

	// TODO: @saturn rings; redraw the same ring, modify the rings diameter via its model matrix

	// ***********************************************************************************

	// TODO: Add space shuttle
	// Bonus: Compute elliptical orbit

	// ***********************************************************************************

    glUseProgram(0);
    glutSwapBuffers();

}

void cleanUp() {
 
    glDeleteTextures(1, &earthTex);
    glDeleteTextures(1, &moonTex);
    glDeleteTextures(1, &saturnTex);
    glDeleteTextures(1, &backgroundTex);
    glDeleteTextures(1, &earthMaskTex);
    glDeleteTextures(1, &blackMaskTex);

    glDeleteProgram(SunShader.Shader);
    glDeleteProgram(TexturePhongShader.Shader);

    glDeleteVertexArrays(1, &geometryShuttle.vao);
    glDeleteVertexArrays(1, &geometryCube.vao);
    glDeleteVertexArrays(1, &geometrySphere.vao);
 
}

void onIdle()
{
    // set the camera:
    V = cam.getView();
    lightSource = V * glm::vec4(0, 0, 0, 1);

    t+= speed;  // increase the time parameter

    glutPostRedisplay();
}

void onMouseDown(int button, int state, int x, int y)
{
    mouseStartPosition = glm::vec2(x, y);
}

void onMouseMove(int x, int y)
{
    int deltaX = x - mouseStartPosition.x;
    int deltaY = y - mouseStartPosition.y;

    cam.yaw(-deltaX * angleDelta);
    cam.pitch(-deltaY * angleDelta);

    mouseStartPosition = glm::vec2(x, y);
}

// the keyboard handler:
void keyboard(unsigned char key, int x, int y)
{

    switch (key)
    {

    case 27:
        cleanUp();
        exit(1);
        break;

    case 'p': // toggle polygon mode:	
        wireframe_mode = !wireframe_mode;
        if (wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;

        // increase / decrease the speed of the point light P:     
    case '+':
        speed += 0.01;
        break;
    case '-':
        speed -= 0.01;
        break;

    case 'w':
        cam.moveForward(forwardDelta);
        break;

    case 's':
        cam.moveBackward(forwardDelta);
        break;

    case 'a':
        cam.roll(angleDelta);
        break;

    case 'd':
        cam.roll(-angleDelta);
        break;
    }


    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(window_widht, window_height);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  
     glutCreateWindow("Planet System");	
     GLenum err = glewInit();
     if (GLEW_OK != err)
     {
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
     }

     glutDisplayFunc(display);

     glutKeyboardFunc(keyboard);
     glutMotionFunc(onMouseMove);
     glutMouseFunc(onMouseDown);

     glutReshapeFunc(reshape);
     glutIdleFunc(onIdle);

     initGL();

     glutMainLoop();
}
