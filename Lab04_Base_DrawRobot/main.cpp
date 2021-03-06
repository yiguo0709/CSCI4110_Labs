#include <string>
#include <iostream>
#include <fstream>
#include <stack>
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

int windowId = 0;

// projection matrix - perspective projection
glm::mat4 projectionMatrix;

// view matrix - orient everything around our preferred view
glm::mat4 viewMatrix;

GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;

bool isAnimating = true;
int frame_delay = 500;
int keyFrame = 1;
int lastFrameMillis = 0;
float times = 0;

float yRotationSpeed = 0.1f;
float yRotation = -85.0f;

void drawCube(glm::mat4 modelMatrix, glm::vec4 colour);
static GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
static GLuint createShader(const GLenum shaderType, const std::string shaderSource);

static const GLfloat vertexPositionData[] = {
    -1.0f, -1.0f,  1.0f,  // front
     1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  // back
     1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f
};
static const GLushort indexData[] = {
  0, 1, 2,   // front
  3, 2, 1,
  5, 6, 7,   // back
  5, 4, 6,
  1, 7, 3,   // right
  1, 5, 7,
  4, 0, 2,   // left
  4, 2, 6,
  2, 7, 6,   // top
  2, 3, 7,
  0, 4, 5,   // bottom
  0, 5, 1
};
int numVertices = 36;

static void createGeometry(void) {
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionData), vertexPositionData, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
}

// TODO:  Add some key frame data for rotating and translating each of the body parts


static void update(void) {
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    times = ((float)milliseconds / 1000.f) * (isAnimating ? 1.0f : 0.0f) * 5.0f;

    // rotate the entire model, to that we can examine it
    yRotation += yRotationSpeed;

    // update the bones
    if (isAnimating && ((milliseconds - lastFrameMillis) > frame_delay)) {
      lastFrameMillis = milliseconds;
      keyFrame++;

      if (keyFrame > 7) {
        keyFrame = 0;
      }
    }

    glutPostRedisplay();
}

static void render(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // turn on depth buffering
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH);
  //glDepthFunc(GL_ALWAYS);

  // activate our shader program
  glUseProgram(programId);

  // colours
  glm::vec4 red(0.8, 0.0, 0.0, 1.0);
  glm::vec4 green(0.0, 0.8, 0.0, 1.0);
  glm::vec4 blue(0.0, 0.0, 0.8, 1.0);
  glm::vec4 yellow(1.0, 1.0,0.0,1.0);

  glm::mat4 baseMatrix = glm::mat4(1.0f);
  baseMatrix = glm::rotate(baseMatrix, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f));

  // Reminder:  Use this order for transforms:  scale, rotation, translation

  glm::mat4 torso, upperRLeg, lowerRLeg, upperLLeg, lowerLLeg, leftArm, rightArm, head, closeLArm, closeRArm, farRArm, farLArm;

  torso = baseMatrix;
  torso = glm::translate(torso, glm::vec3(0.0f, 4.0f * sin(times) * 0.25, 0.0f));
  drawCube(glm::scale(torso, glm::vec3(3.0f, 3.0f, 1.0f)), red);

  head = torso;
  head = glm::translate(head, glm::vec3(0.0f, 4.8f, 0.0f));
  drawCube(glm::scale(head, glm::vec3(2.0,1.5f, 1.0f)), green);

  upperRLeg = torso; //No scale from the Torso here
  upperRLeg = glm::translate(upperRLeg, glm::vec3(2.0f, -4.5f, 0.0f));//the pivot
  upperRLeg = glm::rotate(upperRLeg, glm::radians(-30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(upperRLeg,glm::vec3(0.95f)),yellow);
  upperRLeg = glm::translate(upperRLeg, glm::vec3(0.0f, -1.5f, 0.0f));
  drawCube(glm::scale(upperRLeg, glm::vec3(1.0f, 2.0f, 1.0f)), blue);

  lowerRLeg = upperRLeg;
  lowerRLeg = glm::translate(lowerRLeg, glm::vec3(0.0f, -2.0f, 0.0f));
  lowerRLeg = glm::rotate(lowerRLeg, glm::radians(-30.0f) * sin(times) + glm::radians(-35.0f),glm::vec3(1.0f,0.0f,0.0f));
  drawCube(glm::scale(lowerRLeg,glm::vec3(0.75f)),yellow);
  lowerRLeg = glm::translate(lowerRLeg, glm::vec3(0.0f, -2.0f, 0.0f));
  drawCube(glm::scale(lowerRLeg,glm::vec3(0.8f,2.0f,0.8f)),green);

  upperLLeg = torso; //No scale from the Torso here
  upperLLeg = glm::translate(upperLLeg, glm::vec3(-2.0f, -4.5f,0.0f));//the pivot
  upperLLeg = glm::rotate(upperLLeg, glm::radians(30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(upperLLeg,glm::vec3(0.95f)),yellow);
  upperLLeg = glm::translate(upperLLeg, glm::vec3(0.0f, -1.5f, 0.0f));
  drawCube(glm::scale(upperLLeg, glm::vec3(1.0f, 2.0f, 1.0f)), blue);

  lowerLLeg = upperLLeg;
  lowerLLeg = glm::translate(lowerLLeg, glm::vec3(0.0f, -2.0f, 0.0f));
  lowerLLeg = glm::rotate(lowerLLeg, glm::radians(30.0f) * sin(times) + glm::radians(-35.0f),glm::vec3(1.0f,0.0f,0.0f));
  drawCube(glm::scale(lowerLLeg,glm::vec3(0.75f)),yellow);
  lowerLLeg = glm::translate(lowerLLeg, glm::vec3(0.0f, -2.0f, 0.0f));
  drawCube(glm::scale(lowerLLeg,glm::vec3(0.8f,2.0f,0.8f)),green);

  //Right Brachium
  closeRArm = torso;//no scal from torso here
  closeRArm = glm::translate(closeRArm, glm::vec3(4.0f, 1.0f, 0.0f));//the Pivot
  closeRArm = glm::rotate(closeRArm, glm::radians(-30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(closeRArm, glm::vec3(0.95f)), yellow);
  closeRArm = glm::translate(closeRArm, glm::vec3(0.0, -1.5, 0.0f));
  drawCube(glm::scale(closeRArm, glm::vec3(1.0, 1.7f, 1.0f)), blue);

  farRArm = closeRArm;
  farRArm = glm::translate(farRArm, glm::vec3(0.0, -2.0, 0.0f));
  farRArm = glm::rotate(farRArm, glm::radians(-50.0f) * sin(times) + glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(farRArm, glm::vec3(0.75f)), yellow);
  farRArm = glm::translate(farRArm, glm::vec3(0.0f, -2.0f, 0.0f));
  drawCube(glm::scale(farRArm, glm::vec3(0.8f, 2.0f, 0.8f)), green);

  //Left Arm

  closeLArm = torso;//no scal from torso here
  closeLArm = glm::translate(closeLArm, glm::vec3(-4.0f, 1.0f, 0.0f));//the Pivot
  closeLArm = glm::rotate(closeLArm, glm::radians(30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(closeLArm, glm::vec3(0.95f)), yellow);
  closeLArm = glm::translate(closeLArm, glm::vec3(0.0, -1.5, 0.0f));
  drawCube(glm::scale(closeLArm, glm::vec3(1.0, 1.7f, 1.0f)), blue);


  farLArm = closeLArm;
  farLArm = glm::translate(farLArm, glm::vec3(0.0, -2.0, 0.0f));
  farLArm = glm::rotate(farLArm, glm::radians(50.0f) * sin(times) + glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  drawCube(glm::scale(farLArm, glm::vec3(0.75f)), yellow);
  farLArm = glm::translate(farLArm, glm::vec3(0.0f, -2.0f, 0.0f));
  drawCube(glm::scale(farLArm, glm::vec3(0.8f, 2.0f, 0.8f)), green);

  // make the draw buffer the display buffer (i.e. display what we have drawn)
  glutSwapBuffers();
}

void drawCube(glm::mat4 modelMatrix, glm::vec4 colour) {
  // model-view-projection matrix
  glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
  GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
  glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

  // cube colour
  GLuint colourId = glGetUniformLocation(programId, "u_colour");
  glUniform4fv(colourId, 1, (GLfloat*)&colour[0]);

  // enable the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

  // configure the attribute array (the layout of the vertex buffer)
  glVertexAttribPointer(positionBufferId, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void *)0);
  glEnableVertexAttribArray(positionBufferId);

  // draw the triangle strip
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_SHORT, (void*)0);

  // disable the attribute array
  glDisableVertexAttribArray(positionBufferId);
}

static void reshape(int width, int height) {
    float aspectRatio = (float)width / (float)height;
    projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

    // if using perpsective projection, update projection matrix
    glViewport(0, 0, width, height);
}

static void drag(int x, int y) {
}

static void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    }
}

static void keyboard(unsigned char key, int x, int y) {
    if (key == 'r') {
      if (yRotationSpeed > 0.0) {
        yRotationSpeed = 0.0;
      } else {
        yRotationSpeed = 0.1;
      }
      std::cout << "Toggling rotation, speed: " << yRotationSpeed << std::endl;
    } else if (key == 'a') {
      isAnimating = !isAnimating;
      std::cout << "Toggling animation: " << isAnimating << std::endl;
      std::cout << "Key frame: " << keyFrame << std::endl;
    } else if (key == 27) {
      glutDestroyWindow(windowId);
      exit(0);
    }
    std::cout << "Key pressed: " << key << std::endl;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    windowId = glutCreateWindow("Lab 04");
    glutIdleFunc(&update);
    glutDisplayFunc(&render);
    glutReshapeFunc(&reshape);
    glutMotionFunc(&drag);
    glutMouseFunc(&mouse);
    glutKeyboardFunc(&keyboard);

    glewInit();
    if (!GLEW_VERSION_2_0) {
        std::cerr << "OpenGL 2.0 not available" << std::endl;
        return 1;
    }
    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;

    createGeometry();
    programId = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // create the view matrix (position and orient the camera)
    viewMatrix = glm::lookAt(
        glm::vec3(0,0,25), // eye/camera location
        glm::vec3(0,0,0),    // where to look
        glm::vec3(0,1,0)     // up
    );

    glutMainLoop();

    return 0;
}

static GLuint createShader(const GLenum shaderType, const std::string shaderFilename) {
  // load the shader source code
  std::ifstream fileIn(shaderFilename.c_str());

	if (!fileIn.is_open()) {
		return -1;
	}

	std::string shaderSource;
	std::string line;
	while (getline(fileIn, line)) {
		shaderSource.append(line);
		shaderSource.append("\n");
	}

	const char* sourceCode = shaderSource.c_str();

	// create a shader with the specified source code
	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &sourceCode, nullptr);

	// compile the shader
	glCompileShader(shaderId);

	// check if there were any compilation errors
	int result;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int errorLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLength);
		char *errorMessage = new char[errorLength];

		glGetShaderInfoLog(shaderId, errorLength, &errorLength, errorMessage);
		std::cout << "Shader compilation failed: " << errorMessage << std::endl;

		delete[] errorMessage;

		glDeleteShader(shaderId);

		return 0;
	}

	return shaderId;
}

static GLuint createShaderProgram(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
	// create and compile a shader for each
	GLuint vShaderId = createShader(GL_VERTEX_SHADER, vertexShaderFilename);
	GLuint fShaderId = createShader(GL_FRAGMENT_SHADER, fragmentShaderFilename);

	// create and link the shaders into a program
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vShaderId);
	glAttachShader(programId, fShaderId);
	glLinkProgram(programId);
	glValidateProgram(programId);

	// delete the shaders
	glDetachShader(programId, vShaderId);
	glDetachShader(programId, fShaderId);
	glDeleteShader(vShaderId);
	glDeleteShader(fShaderId);

	return programId;
}
