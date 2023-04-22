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

cy::TriMesh skyboxMesh;

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

std::vector<cy::Vec3f> skyboxVertices;

float windowWidth = 1024;
float windowHeight = 800;

bool leftButtonDown = false;

cy::Vec3f cameraPos;

float cameraX = 0.0f; // x axis camera movements
float cameraY = 0.0f; // y axis camera movements

float prevAngleX, prevAngleY;  //previous x and y value for left click
float currAngleX, currAngleY; //current x and y value for left click

float motionScale = .2;

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
	viewMatrix.SetView(cameraPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
	rotationMatrix.SetRotationXYZ(toRadians(cameraX), toRadians(cameraY), 0.0f);
	//viewMatrix = rotationMatrix * viewMatrix;
	mvpMatrix = projMatrix * viewMatrix * cy::Matrix4f(1.0f) * rotationMatrix;

	GLuint skyboxMVP = glGetUniformLocation(skyboxProgram.GetID(), "mvp");
	glUniformMatrix4fv(skyboxMVP, 1, GL_FALSE, &mvpMatrix(0, 0));


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

	buildSkyboxShaders();

	GLuint skyboxPos = glGetAttribLocation(skyboxProgram.GetID(), "pos");
	glEnableVertexAttribArray(skyboxPos);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(skyboxPos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

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


