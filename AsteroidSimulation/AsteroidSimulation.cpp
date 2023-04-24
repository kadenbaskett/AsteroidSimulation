/*
* Kaden Baskett
* 04/21/2023
* Interactive Computer Graphics
* Final Project - Asteroid Simulation
*/

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/GL.h>
#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"
#include "lodepng.h"


// callbacks
void render();
void keyboard(unsigned char key, int x, int y);
void keyboardSpecial(int key, int x, int y);
void handleMouse(int button, int state, int x, int y);
void handleMouseMotion(int x, int y);
void idle();

// helpers
void initialize();
void update();
void loadSkybox();
void loadAsteroids();
void buildSkyboxShaders();
void buildFirstAsteroidShaders();
void buildSecondAsteroidShaders();
float toRadians(float degrees);
void resetSimulation();
bool checkCollision();
float getModelRadius(const std::vector<cy::Vec3f>& vertices, float scale);

// space skybox enviroment
cy::GLSLProgram skyboxProgram;

cy::TriMesh skyboxMesh;
GLuint skyboxTexture;
GLuint skyboxVAO;
GLuint skyboxVBO;

std::vector<cy::Vec3f> skyboxVertices;

cy::Matrix4f skyboxMVPMatrix;
cy::Matrix4f skyboxViewMatrix;
cy::Matrix4f skyboxProjMatrix;
cy::Matrix4f skyboxRotationMatrix;

unsigned int spaceTexWidth, spaceTexHeight = 1024;

std::vector<unsigned char> spaceFace1;
std::vector<unsigned char> spaceFace2;
std::vector<unsigned char> spaceFace3;
std::vector<unsigned char> spaceFace4;
std::vector<unsigned char> spaceFace5;
std::vector<unsigned char> spaceFace6;

// both asteroids
std::vector<cy::Vec3f> asteroidVertices;
cy::TriMesh asteroidMesh;

cyGLTexture2D asteroidTexture;
std::vector<unsigned char> astroidTextureImage;
unsigned asteroidTextureWidth, asteroidTextureHeight = 2048;

cyGLTexture2D asteroidHeight;
std::vector<unsigned char> astroidHeightImage;
unsigned asteroidHeightWidth, asteroidHeightHeight = 2048;

// asteroid 1
cy::GLSLProgram firstAsteroidProgram;

GLuint firstAsteroidVAO;
GLuint firstAsteroidVBO;

cy::Matrix4f firstAsteroidMVPMatrix;
cy::Matrix4f firstAsteroidViewMatrix;
cy::Matrix4f firstAsteroidProjMatrix;
cy::Matrix4f firstAsteroidRotationMatrix;
cy::Matrix4f firstAsteroidModelMatrix;

float firstAsteroidRadius;

float firstAsteroidScale = .02f;

// asteroid 2
cy::GLSLProgram secondAsteroidProgram;

GLuint secondAsteroidVAO;
GLuint secondAsteroidVBO;

cy::Matrix4f secondAsteroidMVPMatrix;
cy::Matrix4f secondAsteroidViewMatrix;
cy::Matrix4f secondAsteroidProjMatrix;
cy::Matrix4f secondAsteroidRotationMatrix;
cy::Matrix4f secondAsteroidModelMatrix;

float secondAsteroidRadius;

float secondAsteroidScale = .015f;

float radiusScale = 0.65f;

// display window
float windowWidth = 1024;
float windowHeight = 800;

// handle mouse actions
bool leftButtonDown = false;

cy::Vec3f cameraPos;

float cameraX = 0.0f; // x axis camera movements
float cameraY = 0.0f; // y axis camera movements

int prevX, prevY = 0;

float prevAngleX, prevAngleY;  //previous x and y value for left click
float currAngleX, currAngleY; //current x and y value for left click

float motionScale = .2;

bool simulating = false;


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Asteroid Simulation");

	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(handleMouse);
	glutMotionFunc(handleMouseMotion);
	glutIdleFunc(idle);

	initialize();

	glutMainLoop();
	return 0;
}

// callbacks
void render() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LESS);
	glEnable(GL_TEXTURE_2D);

	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	update();

	glDepthMask(GL_FALSE);

	// draw space enviroment sky box
	skyboxProgram.Bind();

	GLuint skyboxMVP = glGetUniformLocation(skyboxProgram.GetID(), "mvp");
	glUniformMatrix4fv(skyboxMVP, 1, GL_FALSE, &skyboxMVPMatrix(0, 0));

	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	// draw first asteroid
	firstAsteroidProgram.Bind();

	GLuint firstAsteroidMVP = glGetUniformLocation(firstAsteroidProgram.GetID(), "mvp");
	glUniformMatrix4fv(firstAsteroidMVP, 1, GL_FALSE, &firstAsteroidMVPMatrix(0, 0));

	glBindVertexArray(firstAsteroidVAO);
	glDrawArrays(GL_TRIANGLES, 0, asteroidVertices.size());

	// draw second asteroid
	secondAsteroidProgram.Bind();

	GLuint secondAsteroidMVP = glGetUniformLocation(secondAsteroidProgram.GetID(), "mvp");
	glUniformMatrix4fv(secondAsteroidMVP, 1, GL_FALSE, &secondAsteroidMVPMatrix(0, 0));

	glBindVertexArray(secondAsteroidVAO);
	glDrawArrays(GL_TRIANGLES, 0, asteroidVertices.size());

	// swap buffers
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0); // handle escape key
	}
	else if (key == ' ') {
		simulating = true; // handle space bar
	}
}

void keyboardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_CTRL_L || key == GLUT_KEY_CTRL_R) {
		simulating = false;
		resetSimulation(); // handle control key
	}
}

void handleMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			prevAngleX = (float)x;
			prevAngleY = (float)y;

			leftButtonDown = true;
		}
		else {
			leftButtonDown = false;
		}
	}
}

void handleMouseMotion(int x, int y) {
	currAngleX = (float)x;
	currAngleY = (float)y;

	// Update view matrix when left mouse button is pressed and dragged
	if (leftButtonDown)
	{
		// Calculate change in mouse position
		int changeX = (currAngleX - prevAngleX) * motionScale;
		int changeY = (currAngleY - prevAngleY) * motionScale;

		// Calculate rotation angles
		prevAngleX = currAngleX;
		prevAngleY = currAngleY;

		// update camera position
		cameraX += changeY;
		cameraY += changeX;

		glutPostRedisplay();
	}
}

void idle() {
	glutPostRedisplay();
}

void resetSimulation() {
	// common matrices /vectors
	cameraPos = cy::Vec3f(0.0f, 0.0f, 10.0f);

	// skybox matrices
	skyboxMVPMatrix = cy::Matrix4f(1.0f);
	skyboxViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	skyboxProjMatrix.SetPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);
	skyboxRotationMatrix.SetRotationXYZ(cameraX, cameraY, 0.0f);

	// first asteroid matrices
	firstAsteroidMVPMatrix = cy::Matrix4f(1.0f);
	firstAsteroidViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	firstAsteroidProjMatrix.SetPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);
	firstAsteroidRotationMatrix.SetRotationXYZ(cameraX, cameraY, 0.0f);
	firstAsteroidModelMatrix = cy::Matrix4f(1.0f);
	firstAsteroidModelMatrix.SetScale(firstAsteroidScale);
	firstAsteroidModelMatrix.AddTranslation(cy::Vec3f(-4.0f, -2.0f, 0.0f));

	// second asteroid matrices
	secondAsteroidMVPMatrix = cy::Matrix4f(1.0f);
	secondAsteroidViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	secondAsteroidProjMatrix.SetPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);
	secondAsteroidRotationMatrix.SetRotationXYZ(cameraX, cameraY, 0.0f);
	secondAsteroidModelMatrix = cy::Matrix4f(1.0f);
	secondAsteroidModelMatrix.SetScale(secondAsteroidScale);
	secondAsteroidModelMatrix.AddTranslation(cy::Vec3f(3.5f, 2.0f, 0.0f));
}
// helpers
void initialize() {
	resetSimulation();

	buildSkyboxShaders();
	buildFirstAsteroidShaders();
	buildSecondAsteroidShaders();

	loadSkybox();
	loadAsteroids();
}

void update() {
	// update skybox matrices
	skyboxViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	skyboxRotationMatrix.SetRotationXYZ(toRadians(cameraX), toRadians(cameraY), 0.0f);
	skyboxMVPMatrix = skyboxProjMatrix * skyboxViewMatrix * cy::Matrix4f(1.0f) * skyboxRotationMatrix;

	// update first asteroid matrices
	firstAsteroidViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	firstAsteroidRotationMatrix.SetRotationXYZ(toRadians(cameraX), toRadians(cameraY), 0.0f);
	firstAsteroidMVPMatrix = firstAsteroidProjMatrix * firstAsteroidViewMatrix * firstAsteroidModelMatrix * firstAsteroidRotationMatrix;

	// update second asteroid matrices
	secondAsteroidViewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	secondAsteroidRotationMatrix.SetRotationXYZ(toRadians(cameraX), toRadians(cameraY), 0.0f);
	secondAsteroidMVPMatrix = secondAsteroidProjMatrix * secondAsteroidViewMatrix * secondAsteroidModelMatrix * secondAsteroidRotationMatrix;

	if (simulating) {
		// move asteroids towards eachother
		firstAsteroidModelMatrix.AddTranslation(cy::Vec3f(0.005f, 0.0025f, 0.0));
		secondAsteroidModelMatrix.AddTranslation(cy::Vec3f(-0.005f, -0.0025f, 0.0));
	}

	if (checkCollision()) {
		// explode asteroids and make smaller pieces
		resetSimulation();
	}
}

void loadSkybox()
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	unsigned err1 = lodepng::decode(spaceFace1, spaceTexWidth, spaceTexHeight, "spaceRight.PNG");
	if (err1) {
		std::cout << "Error loading right space PNG." << std::endl;
	}
	unsigned err2 = lodepng::decode(spaceFace2, spaceTexWidth, spaceTexHeight, "spaceLeft.PNG");
	if (err2) {
		std::cout << "Error loading left space PNG." << std::endl;
	}
	unsigned err3 = lodepng::decode(spaceFace3, spaceTexWidth, spaceTexHeight, "spaceUp.PNG");
	if (err3) {
		std::cout << "Error loading up space PNG." << std::endl;
	}
	unsigned err4 = lodepng::decode(spaceFace4, spaceTexWidth, spaceTexHeight, "spaceDown.PNG");
	if (err4) {
		std::cout << "Error loading down space PNG." << std::endl;
	}
	unsigned err5 = lodepng::decode(spaceFace5, spaceTexWidth, spaceTexHeight, "spaceFront.PNG");
	if (err5) {
		std::cout << "Error loading back space PNG." << std::endl;
	}
	unsigned err6 = lodepng::decode(spaceFace6, spaceTexWidth, spaceTexHeight, "spaceBack.PNG");
	if (err6) {
		std::cout << "Error loading front space PNG." << std::endl;
	}

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace1[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace2[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace3[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace4[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace5[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &spaceFace6[0]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	skyboxMesh.LoadFromFileObj("cube.obj");
	skyboxMesh.ComputeNormals();

	for (int i = 0; i < skyboxMesh.NF(); i++) {
		skyboxVertices.push_back(skyboxMesh.V(skyboxMesh.F(i).v[0])); //store vertex 1
		skyboxVertices.push_back(skyboxMesh.V(skyboxMesh.F(i).v[1])); //store vertex 2
		skyboxVertices.push_back(skyboxMesh.V(skyboxMesh.F(i).v[2])); //store vertex 3
	}

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * skyboxVertices.size(), &skyboxVertices[0], GL_STATIC_DRAW);

	GLuint skyboxPos = glGetAttribLocation(skyboxProgram.GetID(), "pos");
	glEnableVertexAttribArray(skyboxPos);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(skyboxPos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	std::cout << "Finished loading space enviroment sky box." << std::endl;
}

void loadAsteroids()
{
	unsigned err1 = lodepng::decode(astroidTextureImage, asteroidTextureWidth, asteroidTextureHeight, "asteroidTexture.PNG");
	if (err1) {
		std::cout << "Error decoding asteroid texture png." << std::endl;
	}

	unsigned err2 = lodepng::decode(astroidHeightImage, asteroidHeightWidth, asteroidHeightHeight, "asteroidHeight.PNG");
	if (err2) {
		std::cout << "Error decoding asteroid height png." << std::endl;
	}

	asteroidTexture.Initialize();
	asteroidHeight.Initialize();
	asteroidTexture.SetImage(&astroidTextureImage[0], 4, asteroidTextureWidth, asteroidTextureHeight);
	asteroidHeight.SetImage(&astroidHeightImage[0], 4, asteroidHeightWidth, asteroidHeightHeight);
	asteroidTexture.BuildMipmaps();
	asteroidHeight.BuildMipmaps();
	asteroidTexture.Bind(1);
	asteroidHeight.Bind(2);

	asteroidMesh.LoadFromFileObj("asteroid.obj");
	asteroidMesh.ComputeNormals();

	for (int i = 0; i < asteroidMesh.NF(); i++) {
		asteroidVertices.push_back(asteroidMesh.V(asteroidMesh.F(i).v[0])); //store vertex 1
		asteroidVertices.push_back(asteroidMesh.V(asteroidMesh.F(i).v[1])); //store vertex 2
		asteroidVertices.push_back(asteroidMesh.V(asteroidMesh.F(i).v[2])); //store vertex 3
	}

	firstAsteroidRadius = getModelRadius(asteroidVertices, firstAsteroidScale);
	secondAsteroidRadius = getModelRadius(asteroidVertices, secondAsteroidScale);

	// first asteroid
	glGenVertexArrays(1, &firstAsteroidVAO);
	glBindVertexArray(firstAsteroidVAO);

	glGenBuffers(1, &firstAsteroidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, firstAsteroidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * asteroidVertices.size(), &asteroidVertices[0], GL_STATIC_DRAW);

	GLuint firstAsteroidPos = glGetAttribLocation(firstAsteroidProgram.GetID(), "pos");
	glEnableVertexAttribArray(firstAsteroidPos);
	glBindBuffer(GL_ARRAY_BUFFER, firstAsteroidVBO);
	glVertexAttribPointer(firstAsteroidPos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// second asteroid
	glGenVertexArrays(1, &secondAsteroidVAO);
	glBindVertexArray(secondAsteroidVAO);

	glGenBuffers(1, &secondAsteroidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, secondAsteroidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * asteroidVertices.size(), &asteroidVertices[0], GL_STATIC_DRAW);

	GLuint secondAsteroidPos = glGetAttribLocation(secondAsteroidProgram.GetID(), "pos");
	glEnableVertexAttribArray(secondAsteroidPos);
	glBindBuffer(GL_ARRAY_BUFFER, firstAsteroidVBO);
	glVertexAttribPointer(secondAsteroidPos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	firstAsteroidProgram["asteroidTexture"] = 1;
	secondAsteroidProgram["asteroidTexture"] = 1;

	firstAsteroidProgram["asteroidDisplacement"] = 2;
	secondAsteroidProgram["asteroidDisplacement"] = 2;

	std::cout << "Finished loading asteroids." << std::endl;
}

bool checkCollision() {
	cy::Vec3f firstAsteroidCenter = firstAsteroidModelMatrix.GetTranslation();
	cy::Vec3f secondAsteroidCenter = secondAsteroidModelMatrix.GetTranslation();

	// compute distance between model'c centers
	float dx = secondAsteroidCenter.x - firstAsteroidCenter.x;
	float dy = secondAsteroidCenter.y - firstAsteroidCenter.y;
	float dz = secondAsteroidCenter.z - firstAsteroidCenter.z;
	float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

	float radiusSum = firstAsteroidRadius + secondAsteroidRadius;

	if (distance <= radiusSum) {
		// asteroids are colliding
		return true;
	}
	else {
		// asteroids are NOT colliding
		return false;
	}
}

float getModelRadius(const std::vector<cy::Vec3f>& vertices, float scale) {
	float radius = 0.0f;

	for (const cy::Vec3f& vertex : vertices) {
		float distance = scale * vertex.Length();
		if (distance > radius) {
			radius = distance;
		}
	}

	return radius * radiusScale;
}

void buildSkyboxShaders() {
	bool skyboxShadersCompiled = skyboxProgram.BuildFiles("spaceEnv.vert", "spaceEnv.frag");
	if (!skyboxShadersCompiled) {
		std::cout << "Skybox shaders failed to compile!" << std::endl;
	}
}

void buildFirstAsteroidShaders() {
	bool firstAsteroidShadersCompiled = firstAsteroidProgram.BuildFiles("asteroid1.vert", "asteroid1.frag");
	if (!firstAsteroidShadersCompiled) {
		std::cout << "First asteroid shaders failed to compile!" << std::endl;
	}
}

void buildSecondAsteroidShaders() {
	bool secondAsteroidShadersCompiled = secondAsteroidProgram.BuildFiles("asteroid2.vert", "asteroid2.frag");
	if (!secondAsteroidShadersCompiled) {
		std::cout << "Second asteroid shaders failed to compile!" << std::endl;
	}
}

float toRadians(float degrees) {
	return degrees * (3.41159264 / 180);
}


