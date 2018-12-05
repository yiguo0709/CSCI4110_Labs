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
#include <cmath>

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

const float G = pow(6.674, -11);
const float Me = pow(5.972, 24);
const float Mm = pow(7.34767309, 22);
const float r = 384400000.0f;
const float timeScale = 1000000000.0f;

// Earth
glm::vec3 earthPosition;
glm::vec3 earthVelocity;
float earthRadius = 6371000.0f; // M

// Moon
glm::vec3 moonPosition;
glm::vec3 moonVelocity;
float moonRadius = 1737000.0f; // M

float gravitationalForce(float massA, float massB, float radius) {
	return (G * massA * massB) / (radius * radius);
}

float acceleration(float mass, float force) {
	return force / mass;
}

glm::vec3 velocity(glm::vec3 direction, float acceleration, float dt) {
	return direction * acceleration * dt * timeScale;
}

float previousTime = 0.0f;
float deltaTime = 0.0f;
static void update(void) {
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);

    float time = (float) milliseconds / 1000.0f; // Creates time in seconds
    deltaTime = time - previousTime;
    previousTime = time;

   	float distance = glm::length(earthPosition - moonPosition);

    float gravityForce = gravitationalForce(Me, Mm, distance); // Same for both

    // Earth
    float accelerationEarth = acceleration(Me, gravityForce);
    earthVelocity += velocity(glm::normalize(moonPosition-earthPosition), accelerationEarth, deltaTime);

    // Moon
    float accelerationMoon = acceleration(Mm, gravityForce);
    moonVelocity += velocity(glm::normalize(earthPosition-moonPosition), accelerationMoon, deltaTime);

    earthPosition += earthVelocity;
    moonPosition += moonVelocity;

    std::cout << "Earth Vel: " << glm::length(earthVelocity) << ", Moon Vel: " << glm::length(moonVelocity) << "\n";

    glutPostRedisplay();
}

static void render(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // activate our shader program
	glUseProgram(programId);

  // turn on depth buffering
  glEnable(GL_DEPTH_TEST);

  // projection matrix - perspective projection
  // Aspect ratio:  4:3 ratio
  float ar = (float) width / (float) height; // Makes the window wider
  // Clipping planes were clipping through the earth, so making it 5.5f instead of 1.5f fixes that
  glm::mat4 projection = glm::ortho(-r * ar, r * ar, -r, r, -earthRadius * 5.5f, earthRadius * 5.5f);
  float screenSize = r * 1.2f;

  // view matrix - orient everything around our preferred view
  glm::mat4 view = glm::mat4(1.0f); // Identity matrix

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

  { // Draw Earth
  	// model matrix: translate, scale, and rotate the model
  	glm::mat4 model = glm::mat4(1.0f);
  	model = glm::translate(model, earthPosition);
  	model = glm::scale(model, glm::vec3(earthRadius, earthRadius, earthRadius) * 5.0f);

  	// model-view-projection matrix
  	glm::mat4 mvp = projection * view * model;
  	GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
  	glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

  	// draw the triangles
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);
  }

  { // Draw Moon
  	// model matrix: translate, scale, and rotate the model
  	glm::mat4 model = glm::mat4(1.0f);
  	model = glm::translate(model, moonPosition);
  	model = glm::scale(model, glm::vec3(moonRadius, moonRadius, moonRadius) * 5.0f);

  	// model-view-projection matrix
  	glm::mat4 mvp = projection * view * model;
  	GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
  	glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

  	// draw the triangles
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);
  }

  // for testing purposes
  //glutSolidTorus(0.5f, 1.5f, 12, 10);

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

    } else if (key == 'r') {

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

	// Initialize the simulation variables
	earthPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	earthVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	moonPosition = glm::vec3(r, 0.0f, 0.0f);
	moonVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

    createGeometry();
    ShaderProgram program;
    program.loadShaders("shaders/flat_vertex.glsl", "shaders/flat_fragment.glsl");
  	programId = program.getProgramId();

    glutMainLoop();

    return 0;
}
