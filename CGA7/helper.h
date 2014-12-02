// this header implements the shader handling.

#include <GL/glew.h> 
#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <math.h>


using namespace std;

// this function creates Shader Program which consists of both, a vertex and a fragment shader

  void createProgram_VF(const char *VSfile, const char *FSfile, GLuint *handle) {
    
    GLint compiled;
    char infoLog[4096];
    int infoLogLength;
    
    filebuf *pbuf_VS,*pbuf_FS;
    ifstream filestr_VS,filestr_FS;
    int size_VS,size_FS;
    char *buffer_VS,*buffer_FS;
    
    filestr_VS.open(VSfile);
    filestr_FS.open(FSfile);
    
    
    
    // get pointer to associated buffer object
    pbuf_VS=filestr_VS.rdbuf();
    pbuf_FS=filestr_FS.rdbuf();
    size_VS=pbuf_VS->pubseekoff (0,ios::end,ios::in);
    size_FS=pbuf_FS->pubseekoff(0,ios::end,ios::in);
    
    pbuf_VS->pubseekpos (0,ios::in);
    pbuf_FS->pubseekpos (0,ios::in);
  
    // allocate memory to contain file data
    buffer_VS=new char[size_VS];
    buffer_FS=new char[size_FS];
    pbuf_VS->sgetn(buffer_VS,size_VS);
    pbuf_FS->sgetn(buffer_FS,size_FS);
    
    const char *VshaderCode = buffer_VS;
    const char *FshaderCode = buffer_FS;
    
    filestr_VS.close();
    filestr_FS.close();
    
    //compile vertex shader: 	
    GLuint Vshader= glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(Vshader,1,&VshaderCode,&size_VS);
    glCompileShader(Vshader);
    glGetShaderiv(Vshader,GL_COMPILE_STATUS, &compiled);
    if ( !compiled) {
      // Print out the info log
      glGetShaderInfoLog(Vshader, sizeof(infoLog), &infoLogLength, infoLog);
      if(infoLogLength > 0)
      {
	printf("CompileShader() infoLog for Vertex Shader %s \n%s\n",VSfile, infoLog);
	exit(1);
      }
    }	

  //compile Fragment shader: 	
    GLuint Fshader= glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(Fshader,1,&FshaderCode,&size_FS);
    glCompileShader(Fshader);
    glGetShaderiv(Fshader,GL_COMPILE_STATUS, &compiled);
    if ( !compiled) {
      // Print out the info log
      glGetShaderInfoLog(Fshader, sizeof(infoLog), &infoLogLength, infoLog);
      if(infoLogLength > 0)
      {
	printf("CompileShader() infoLog for Fragment Shader %s\n%s\n", FSfile, infoLog);
	exit(1);
      }
    }	

    *handle = glCreateProgram();
    glAttachShader(*handle, Vshader);
    glAttachShader(*handle, Fshader);
    
    
    glDeleteShader(Vshader);
    glDeleteShader(Fshader);
    delete buffer_VS;
    delete buffer_FS;
    glLinkProgram(*handle);
  }

// this function creates Shader Program which consists only of a vertex shader

  void createProgram_V(const char *VSfile, GLuint *handle) {
    
    GLint compiled;
    char infoLog[4096];
    int infoLogLength;
    
    filebuf *pbuf;
    ifstream filestr;
    int size;
    char *buffer;
    
    filestr.open(VSfile);
    
    // get pointer to associated buffer object
    pbuf=filestr.rdbuf();
    size=pbuf->pubseekoff (0,ios::end,ios::in);
    pbuf->pubseekpos (0,ios::in);
    
    // allocate memory to contain file data
    buffer=new char[size];
    pbuf->sgetn(buffer,size);
    
    const char *VshaderCode = buffer;
    
    filestr.close();
    
    //compile vertex shader: 	
    GLuint Vshader= glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(Vshader,1,&VshaderCode,&size);
    glCompileShader(Vshader);
    glGetShaderiv(Vshader,GL_COMPILE_STATUS, &compiled);
    if ( !compiled) {
      // Print out the info log
      glGetShaderInfoLog(Vshader, sizeof(infoLog), &infoLogLength, infoLog);
      if(infoLogLength > 0)
      {
	printf("CompileShader() infoLog %s \n%s\n",VSfile, infoLog);
	exit(1);
      }
    }	
    
    *handle = glCreateProgram();
    glAttachShader(*handle, Vshader);
    
    glDeleteShader(Vshader);
    delete buffer;
    glLinkProgram(*handle);
  }
