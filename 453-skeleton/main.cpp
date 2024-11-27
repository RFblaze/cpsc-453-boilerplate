//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitCube.h"

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}
	Camera camera;
private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

std::vector<glm::vec3> createSurfaceOfRevolution(const std::vector<glm::vec3>& CurvePoints, int numSlices) {
    std::vector<glm::vec3> surfaceVertices;
    float angleStep = glm::radians(360.0f / numSlices);

    // For each point in the original curve
    for (size_t i = 0; i < CurvePoints.size() - 1; ++i) {
        glm::vec3 bottom1 = CurvePoints[i];
        glm::vec3 bottom2 = CurvePoints[i + 1];

        // Create triangles for each slice around the revolution axis
        for (int slice = 0; slice < numSlices; ++slice) {
            float angle1 = slice * angleStep;
            float angle2 = (slice + 1) * angleStep;

            // Rotate points around Y-axis
            glm::vec3 p1(bottom1.x * cos(angle1), bottom1.y, bottom1.x * sin(angle1));
            glm::vec3 p2(bottom1.x * cos(angle2), bottom1.y, bottom1.x * sin(angle2));
            glm::vec3 p3(bottom2.x * cos(angle1), bottom2.y, bottom2.x * sin(angle1));
            glm::vec3 p4(bottom2.x * cos(angle2), bottom2.y, bottom2.x * sin(angle2));

            // First triangle of the quad
            surfaceVertices.push_back(p1);
            surfaceVertices.push_back(p2);
            surfaceVertices.push_back(p3);

            // Second triangle of the quad
            surfaceVertices.push_back(p3);
            surfaceVertices.push_back(p2);
            surfaceVertices.push_back(p4);
        }
    }
    return surfaceVertices;
}

std::vector<glm::vec2> createTextureCoordinates(const std::vector<glm::vec3>& CurvePoints, int numSlices) {
    std::vector<glm::vec2> texCoords;

    size_t numCurvePoints = CurvePoints.size();
    float uStep = 1.0f / float(numSlices);          // Step size for the u-coordinate
    float vStep = 1.0f / float(numCurvePoints - 1); // Step size for the v-coordinate

    // Loop through all points in the curve
    for (size_t i = 0; i < numCurvePoints - 1; ++i) {
        float v1 = i * vStep;           // v-coordinate for the current point
        float v2 = (i + 1) * vStep;     // v-coordinate for the next point

        // Loop through slices
        for (int slice = 0; slice < numSlices; ++slice) {
            float u1 = slice * uStep;           // u-coordinate for the current slice
            float u2 = (slice + 1) * uStep;     // u-coordinate for the next slice

            // Texture coordinates for the two triangles forming the quad
            texCoords.push_back(glm::vec2(u1, v1)); // Bottom-left
            texCoords.push_back(glm::vec2(u2, v1)); // Bottom-right
            texCoords.push_back(glm::vec2(u1, v2)); // Top-left

            texCoords.push_back(glm::vec2(u1, v2)); // Top-left
            texCoords.push_back(glm::vec2(u2, v1)); // Bottom-right
            texCoords.push_back(glm::vec2(u2, v2)); // Top-right
        }
    }

    return texCoords;
}

struct CelestialBody{
	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position = {0.f,0.f,0.f};
	glm::vec3 axis_tilt = {0.f,0.f,0.f};

	glm::mat4 scale = glm::mat4(1.f);
	glm::mat4 tilt;
	glm::mat4 rotation;
	glm::mat4 translation;

	// Tutorial
	// glm::mat4 Mscale;
	// glm::mat4 Mb; // axis tilt, axis rotation
	// Mb = Rotation of axis tilt * Axis rotation (planet rotation)
	// glm::mat4 Mo
	// Mo = Rotation of orbit about the inclination * Translation of p0 (p0 is the planet centre)

	// p0 = Rotation orbit * Translation from the centre of parent to the centre of local

	std::vector<CelestialBody*> children;

	CelestialBody(std::string texturePath, GLint interpolation, std::vector<glm::vec3> sphere, std::vector<glm::vec3> cols) : 
		texture(texturePath, interpolation){
			cgeom.verts = sphere;
			cgeom.cols = cols;
			cgeom.normals = sphere;

			// cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
			// cgeom.texCoords.push_back(glm::vec2(0.f, 0.f));
			// cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
			// cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
			// cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
			// cgeom.texCoords.push_back(glm::vec2(1.f, 1.f));

			ggeom.setVerts(sphere);
			ggeom.setCols(cols);
			ggeom.setNormals(sphere);
			ggeom.setTexCoords(cgeom.texCoords);

	}

	glm::mat4 getLocalMatrix(){
		return translation * tilt * rotation * scale;
	}

	void updateChildren(){
		// runs after the parent has been updated to update the children, and then its children, and so on
		for (int i = 0; i < children.size(); i++){
			children.at(i)->updateLocal();
		}
	}

	void updateLocal(){
		// update the local transformation matrix(ces)

		// after the local has been done, update the children
		updateChildren();
	}
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 4");

	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	UnitCube cube;
	cube.generateGeometry();

	float root2over2 = powf(2, 0.5) / 2.f;
	float root3over2 = powf(3, 0.5) / 2.f;
	float cos15 = (powf(3, 0.5) + 1) / (2 * powf(2, 0.5)); // 0.9659...
	float sin15 = (powf(3, 0.5) - 1) / (2 * powf(2, 0.5)); // 0.2588...

	std::vector<glm::vec3> sphereCoords = std::vector<glm::vec3>();
	sphereCoords.push_back(glm::vec3(0.f,-1.f,0.f));
	sphereCoords.push_back(glm::vec3(sin15,-cos15,0.f));
	sphereCoords.push_back(glm::vec3(0.5f,-root3over2,0.f));
	sphereCoords.push_back(glm::vec3(root2over2,-root2over2,0.f));
	sphereCoords.push_back(glm::vec3(root3over2,-0.5f,0.f));
	sphereCoords.push_back(glm::vec3(cos15,-sin15,0.f));
	sphereCoords.push_back(glm::vec3(1.f,0.f,0.f));
	sphereCoords.push_back(glm::vec3(cos15,sin15,0.f));
	sphereCoords.push_back(glm::vec3(root3over2,0.5f,0.f));
	sphereCoords.push_back(glm::vec3(root2over2,root2over2,0.f));
	sphereCoords.push_back(glm::vec3(0.5f,root3over2,0.f));
	sphereCoords.push_back(glm::vec3(sin15,cos15,0.f));
	sphereCoords.push_back(glm::vec3(0.f,1.f,0.f));

	// the number of slices is chosen based on the # of slices in the semi unit circle * 2 because it makes the subdivisions along every side equal
	std::vector<glm::vec3> Sphere3D = createSurfaceOfRevolution(sphereCoords, 24);

	std::vector<glm::vec3> cols = std::vector<glm::vec3>();
	for (int i = 0; i < int(Sphere3D.size()); i++){
		cols.push_back(glm::vec3(0.f,0.f,0.f));
	}

	CelestialBody sun = CelestialBody("textures/2k_sun.jpg", GL_LINEAR, Sphere3D, cols);
	CelestialBody skybox = CelestialBody("textures/2k_stars.jpg", GL_LINEAR, Sphere3D, cols);
	CelestialBody earth = CelestialBody("textures/2k_earth_daymap.jpg", GL_LINEAR, Sphere3D, cols);
	CelestialBody moon = CelestialBody("textures/2k_moon.jpg", GL_LINEAR, Sphere3D, cols);

	sun.cgeom.texCoords = createTextureCoordinates(sphereCoords, 24);
	sun.ggeom.setTexCoords(sun.cgeom.texCoords);

	for (size_t i = 0; i < sun.cgeom.texCoords.size(); ++i) {
    	std::cout << "TexCoord[" << i << "]: (" << sun.cgeom.texCoords[i].x << ", " << sun.cgeom.texCoords[i].y << ")\n";
	}

	// Define transformation hierarchies so that the transformations are relative to parents
	sun.children.push_back(&earth);
	earth.children.push_back(&moon);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);

		shader.use();

		a4->viewPipeline(shader);

		// cube.m_gpu_geom.bind();
		// glDrawArrays(GL_TRIANGLES, 0, GLsizei(cube.m_size));

		// My attempt at rendering
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		sun.texture.bind();

		sun.ggeom.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sun.cgeom.verts.size()));
		sun.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
