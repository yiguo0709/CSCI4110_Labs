#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/glew.h>
#include <vector>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>


GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;


glm::mat4 projection;
glm::mat4 view;


int startTime = 0;

float xOffset = 0.0f;
float yOffset = 0.0f;
float xAngle = 0.0f;
float zAngle = 0.0f;

static GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
static GLuint createShader(const GLenum shaderType, const std::string shaderSource);
static void drawShip(glm::vec3 position, glm::vec3 direction);

static const GLfloat vertexPositionData[] = {
	 0.0f, -0.5f, 0.0f,
	 0.0f,  1.0f, 0.0f,
	-0.5f, -1.0f, 0.0f,
	 0.5f, -1.0f, 0.0f
};
static const GLushort indexData[] = { 0, 3, 1, 0, 1, 2 };
int numVertices = 6;

static void createGeometry(void) {
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionData), vertexPositionData, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
}
class BOID {
public:
	BOID(glm::vec3 a_position) :
		position(a_position)
	{
		velocity = glm::normalize(a_position);
	}
	void CapVelocity(float maxSpeed)
	{
		if (glm::length(velocity) > maxSpeed)
		{
			velocity *= 0.98f; // glm::normalize(velocity)* maxSpeed;
		}
	}
	void ApplySeperation(glm::vec3 seperationVector)
	{
		velocity += seperationVector * seperationFactor;
	}
	void ApplyCohesion(glm::vec3 cohesionVector)
	{
		velocity += cohesionVector * cohesionFactor;
	}
	void ApplyAlignment(glm::vec3 alignmentVector)
	{
		velocity += alignmentVector * alignmentFactor;
	}
	void ApplyTargetForce(glm::vec3 targetPosition, float targetFactor) {
		glm::vec3 directionVector = (targetPosition - position);
		velocity += directionVector * targetFactor;
	}

	float seperationFactor = 0.4f;
	float cohesionFactor = 0.3f;
	float alignmentFactor = 0.3f;


	glm::vec3 velocity;
	glm::vec3 position;


};
class BOIDManager
{
public:
	BOIDManager(int a_numBoids)
	{
		for (int i = 0; i < a_numBoids; i++) {
			glm::vec3 startingPosition = glm::vec3
			(
				cos((float)i),
				sin((float)i),
				0.0f
			);
			BOID boid(startingPosition);
			boids.push_back(boid);
		}
	}
	glm::vec3 GetLocalPosition(glm::vec3 centerPosition, float radius)
	{
		glm::vec3 averagePosition = glm::vec3(0.0f);
		float count = 0.0f;
		for (int i = 0; i < boids.size(); i++) {
			if (glm::length(boids[i].position - centerPosition) < radius)
			{
				averagePosition += boids[i].position;
				count += 1.0f;
			}
		}

		if (count == 0.0f) { return glm::vec3(0.0f); }
		return averagePosition / count;

	}
	glm::vec3 GetLocalAlignment(glm::vec3 centerPosition, float radius)
	{
		glm::vec3 averageAlignment = glm::vec3(0.0f);
		float count = 0.0f;
		for (int i = 0; i < boids.size(); i++) {
			if (glm::length(boids[i].position - centerPosition) < radius)
			{
				glm::vec3 direction = glm::normalize(boids[i].velocity);
				averageAlignment += direction;
				count += 1.0f;
			}
		}
		if (count == 0.0f) return glm::vec3(0.0f);
		return averageAlignment / count;
	}
	void UpdateBoids(float dt) {
		for (int i = 0; i < boids.size(); i++) {

			glm::vec3 alignmentVector = GetLocalAlignment(boids[i].position, 2.5f);
			glm::vec3 seperationVector = boids[i].position - GetLocalPosition(boids[i].position, 2.5f);
			glm::vec3 cohesionVector = GetLocalPosition(boids[i].position, 2.5f) - boids[i].position;


			boids[i].ApplySeperation(seperationVector);
			// boids[i].ApplyCohesion(cohesionVector);
			// boids[i].ApplyAlignment(alignmentVector);

			boids[i].ApplyTargetForce(glm::vec3(0.0f, 0.0f, 0.0f), 0.01f);
			boids[i].position += boids[i].velocity * dt;
			boids[i].CapVelocity(10.0f);

			// if (boids[i].position.x > 10.0f) boids[i].position.x = -10.0f;
			// if (boids[i].position.x < -10.0f) boids[i].position.x = 10.0f;
			// if (boids[i].position.y > 10.0f) boids[i].position.y = -10.0f;
			// if (boids[i].position.y < -10.0f) boids[i].position.y = 10.0f;

		}
	}

	std::vector<BOID>boids;
};

BOIDManager* manager;
float deltaTime = 0.0f;
float previousFrameTime = 0.0f;


static void update(void) {
	float time = (float)glutGet(GLUT_ELAPSED_TIME) / 1200.0f;
	deltaTime = time - previousFrameTime;
	previousFrameTime = time;

	manager->UpdateBoids(deltaTime);

	glutPostRedisplay();
}

static void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate our shader program
	glUseProgram(programId);

	// turn on depth buffering
	glEnable(GL_DEPTH_TEST);

	// draw the ship
	for (int i = 0; i < manager->boids.size(); i++) {
		BOID b = manager->boids[i];
		drawShip(b.position, glm::normalize(b.velocity));
	}

	// disable the attribute array
	glDisableVertexAttribArray(positionBufferId);

	// make the draw buffer to display buffer (i.e. display what we have drawn)
	glutSwapBuffers();
}

static void drawShip(glm::vec3 position, glm::vec3 direction) {

	float angle = glm::atan(direction.y, direction.x) + glm::radians(270.0f);

	//Create model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));


	// model-view-projection matrix
	glm::mat4 mvp = projection * view * model;
	GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
	glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

	// enable the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	// configure the attribute array (the layout of the vertex buffer)
	glVertexAttribPointer(positionBufferId, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void *)0);
	glEnableVertexAttribArray(positionBufferId);

	// draw the triangle strip
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_SHORT, (void*)0);
}

static void reshape(int width, int height) {
	// if using perpsective projection, update projection matrix
	glViewport(0, 0, width, height);

	// projection matrix - perspective projection
	// FOV:           45°
	// Aspect ratio:  4:3 ratio
	// Z range:       between 0.1 and 100.0
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
	//projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1000.0f, 1000.0f);

}

static void drag(int x, int y) {
}

static void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
	}
}

static void keyboard(unsigned char key, int x, int y) {
	std::cout << "Key pressed: " << key << std::endl;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CSCI 4110u Base OpenGL");
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

	// view matrix - orient everything around our preferred view
	glm::vec3 eyePosition(0, 0, 30);
	view = glm::lookAt(
		eyePosition,
		glm::vec3(0, 0, 0),    // where to look
		glm::vec3(0, 1, 0)     // up
	);

	manager = new BOIDManager(30);

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
