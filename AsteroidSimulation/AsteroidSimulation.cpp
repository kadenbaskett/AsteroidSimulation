/*
* Kaden Baskett
* 04/06/2023
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
void handleMouse(int button, int state, int x, int y);
void handleMouseMotion(int x, int y);
void idle();

// helpers
void initialize();
unsigned int loadSkybox();
void buildSkyboxShaders();
void updatePhysics();
float toRadians(float degrees);

// space skybox enviroment
cy::GLSLProgram skyboxProgram;

GLuint skyboxTexture;
GLuint skyboxVAO;
GLuint skyboxVBO;

unsigned int spaceTexWidth, spaceTexHeight = 1024;

std::vector<unsigned char> spaceFace1;
std::vector<unsigned char> spaceFace2;
std::vector<unsigned char> spaceFace3;
std::vector<unsigned char> spaceFace4;
std::vector<unsigned char> spaceFace5;
std::vector<unsigned char> spaceFace6;

float skyboxVertices[] = {
	// Back face
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,

	// Front face
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	// Left face
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,

	// Right face
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,

	// Bottom face
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,

	// Top face
	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
};

float windowWidth = 1024;
float windowHeight = 800;

bool leftButtonDown = false;

cy::Vec3f cameraPos;

float cameraX = 0.0f; // x axis camera movements
float cameraY = 0.0f; // y axis camera movements

float prevAngleX, prevAngleY;  //previous x and y value for left click
float currAngleX, currAngleY; //current x and y value for left click

cy::Matrix4f mvpMatrix;
cy::Matrix4f viewMatrix;
cy::Matrix4f projMatrix;
cy::Matrix4f rotationMatrix;

int prevX, prevY = 0;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Asteroid Simulation");
	//glutFullScreen();

	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
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

	glDepthMask(GL_FALSE);
	skyboxProgram.Bind();

	// ... set view and projection matrix
	//viewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	//rotationMatrix.SetRotationXYZ(toRadians(cameraX), toRadians(cameraY), 0.0f);
	//viewMatrix = rotationMatrix * viewMatrix;
	//mvpMatrix = projMatrix * viewMatrix * cy::Matrix4f(1.0f) * rotationMatrix;

	GLuint skyboxView = glGetUniformLocation(skyboxProgram.GetID(), "view");
	glUniformMatrix4fv(skyboxView, 1, GL_FALSE, &viewMatrix(0, 0));

	GLuint skyboxProjection = glGetUniformLocation(skyboxProgram.GetID(), "projection");
	glUniformMatrix4fv(skyboxProjection, 1, GL_FALSE, &projMatrix(0, 0));

	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	// ... draw rest of the scene

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0); // handle escape key
	}
}

void handleMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			leftButtonDown = true;
		}
		else {
			leftButtonDown = false;
		}
	}
}

void handleMouseMotion(int x, int y) {
	// Update view matrix when left mouse button is pressed and dragged
	if (leftButtonDown)
	{
		// Calculate change in mouse position
		int deltaX = currAngleX - prevX;
		int deltaY = currAngleY - prevY;

		// Calculate rotation angles
		float rotX = deltaX * 0.4f;
		float rotY = deltaY * 0.4f;

		// Apply rotation to rotation matrix
		rotationMatrix.SetRotationXYZ(toRadians(rotY), toRadians(rotX), 0.0f);

		// Update view matrix with rotated matrix
		viewMatrix = rotationMatrix * viewMatrix;

		// Update previous mouse position
		prevX = currAngleX;
		prevY = currAngleY;

		// Update MVP matrix
		mvpMatrix = projMatrix * viewMatrix * cy::Matrix4f(1.0f) * rotationMatrix;

		glutPostRedisplay();
	}
	else
	{
		// Store current mouse position as previous for next frame
		prevX = currAngleX;
		prevY = currAngleY;
	}
}

void idle() {
	glutPostRedisplay();
}

// helpers
void initialize() {
	mvpMatrix = cy::Matrix4f(1.0f);
	cameraPos = cy::Vec3f(0.0f, 0.0f, 3.0f);
	viewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	projMatrix.SetPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);
	rotationMatrix.SetRotationXYZ(cameraX, cameraY, 0.0f);

	skyboxTexture = loadSkybox();
}

GLuint loadSkybox()
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
	unsigned err5 = lodepng::decode(spaceFace5, spaceTexWidth, spaceTexHeight, "spaceBack.PNG");
	if (err5) {
		std::cout << "Error loading back space PNG." << std::endl;
	}
	unsigned err6 = lodepng::decode(spaceFace6, spaceTexWidth, spaceTexHeight, "spaceFront.PNG");
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

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	buildSkyboxShaders();

	GLuint skyboxPos = glGetAttribLocation(skyboxProgram.GetID(), "pos");
	glEnableVertexAttribArray(skyboxPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	return textureID;
}

void buildSkyboxShaders() {
	bool skyboxShadersCompiled = skyboxProgram.BuildFiles("spaceEnv.vert", "spaceEnv.frag");
	if (!skyboxShadersCompiled) {
		std::cout << "Skybox shaders failed to compile!" << std::endl;
	}
}

void updatePhysics() {

}

float toRadians(float degrees) {
	return degrees * (3.41159264 / 180);
}


