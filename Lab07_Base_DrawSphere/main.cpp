#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ShaderProgram.h"
#include "ObjMesh.h"
#include "UVSphere.h"

#define SCALE_FACTOR 2.0f

int width, height;

GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;
GLuint positions_vbo = 0;
GLuint textureCoords_vbo = 0;
GLuint normals_vbo = 0;
GLuint colours_vbo = 0;

unsigned int numVertices;

float moonRotation = 0.0f;
bool isPointLight = false;
glm::vec4 lightPosDir;

static void createGeometry(void) {
  ObjMesh mesh;
  mesh.load("my_sphere.obj", true, true);

  numVertices = mesh.getNumIndexedVertices();
  glm::vec3* vertexPositions = mesh.getIndexedPositions();
  glm::vec2* vertexTextureCoords = mesh.getIndexedTextureCoords();
  glm::vec3* vertexNormals = mesh.getIndexedNormals();

  glGenBuffers(1, &positions_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), vertexPositions, GL_STATIC_DRAW);

  glGenBuffers(1, &textureCoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), vertexTextureCoords, GL_STATIC_DRAW);

  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), vertexNormals, GL_STATIC_DRAW);

  unsigned int* indexData = mesh.getTriangleIndices();
  int numTriangles = mesh.getNumTriangles();

  glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3, indexData, GL_STATIC_DRAW);
}

  float deltaTime = 0.0f;
  float previousTime = 0.0f;
static void update(void) {
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    float time = (float)milliseconds / 1000.0f;
    deltaTime = time - previousTime;
    previousTime = time;

    moonRotation += 1.0 * deltaTime;

    if (isPointLight) {
      //sun position
    }
    else {
      //sun direction
    }

    glutPostRedisplay();
}

static void render(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // activate our shader program
	glUseProgram(programId);

  // turn on depth buffering
  glEnable(GL_DEPTH_TEST);

  // projection matrix - orthographic(non-perspective) projection
  float aspectRatio = (float)width / (float)height;

  glm::mat4 projection = glm::ortho(
    -10.0f * aspectRatio, 10.0f * aspectRatio,
    -10.0f,10.0f,
    -100.0f,100.0f);

  // view matrix - orient everything around our preferred view
  glm::mat4 view = glm::mat4(1.0f);

  // model matrix: translate, scale, and rotate the model
  glm::mat4 moonModelMat = glm::mat4(1.0f);
  //std::cout << moonRotation << std::endl;
  moonModelMat = glm::translate(moonModelMat, glm::vec3(cos(moonRotation) * 9.0f, sin(moonRotation)* 9.0f, 1.0));

  glm::mat4 earthModelMat =glm::mat4(1.0f);
  earthModelMat = glm::scale(earthModelMat,glm::vec3(3.0f));

#pragma region HIDE THIS STUFF
  // find the names (ids) of each vertex attribute
  GLint positionAttribId = glGetAttribLocation(programId, "position");
  GLint textureCoordsAttribId = glGetAttribLocation(programId, "textureCoords");
  GLint normalAttribId = glGetAttribLocation(programId, "normal");

  // provide the vertex positions to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glEnableVertexAttribArray(positionAttribId);
  glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // provide the vertex texture coordinates to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glEnableVertexAttribArray(textureCoordsAttribId);
  glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // provide the vertex normals to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glEnableVertexAttribArray(normalAttribId);
  glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
#pragma endregion

  glm::vec3 moonColor = glm::vec3(0.4f);
  glm::vec3 earthColor = glm::vec3(0.2f, 0.25f, 1.0f);

  ///////////////////////////////////////////////////////////////////Draw moon
  {
    // model-view-projection matrix
    glm::mat4 mvp = projection * view * moonModelMat;
    GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
    glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

    //Color
    GLuint colorId = glGetUniformLocation(programId, "u_color");
    glUniform3fv(colorId, 1, &moonColor[0]);

    //glUniform4fv(colorId, 1, &moonColor[0]);
	  // draw the triangles
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	  glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);
  }
  //////////////////////////////////////////////////////////////////Draw earth
  {
    // model-view-projection matrix
    glm::mat4 mvp = projection * view * earthModelMat;
    GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
    glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

    //Color
    GLuint colorId = glGetUniformLocation(programId, "u_color");
    glUniform3fv(colorId, 1, &earthColor[0]);

	  // draw the triangles
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	  glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);
  }
  //////////////////////////////////////////////////////////////////////
	// disable the attribute arrays
  glDisableVertexAttribArray(positionAttribId);
  glDisableVertexAttribArray(textureCoordsAttribId);
  glDisableVertexAttribArray(normalAttribId);

	// make the draw buffer to display buffer (i.e. display what we have drawn)
	glutSwapBuffers();
}

static void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    width = w;
    height = h;
}

static void keyboard(unsigned char key, int x, int y) {
    std::cout << "Key pressed: " << key << std::endl;
    if (key == 'l') {
      isPointLight = !isPointLight;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CSCI 3090u Shading in OpenGL");
    glutIdleFunc(&update);
    glutDisplayFunc(&render);
    glutReshapeFunc(&reshape);
    glutKeyboardFunc(&keyboard);

    glewInit();
    if (!GLEW_VERSION_2_0) {
        std::cerr << "OpenGL 2.0 not available" << std::endl;
        return 1;
    }
    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;

    createGeometry();
    ShaderProgram program;
    //program.loadShaders("shaders/normal_vertex.glsl", "shaders/normal_fragment.glsl");
    //program.loadShaders("shaders/gouraud_vertex.glsl", "shaders/gouraud_fragment.glsl");
    program.loadShaders("shaders/lamber_vertex.glsl", "shaders/lambert_fragment.glsl");
  	programId = program.getProgramId();

    glutMainLoop();

    return 0;
}
