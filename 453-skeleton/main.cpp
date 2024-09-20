#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"


// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

private:
	ShaderProgram& shader;
};



int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	// vertices
	// middle
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// right
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(1.f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// left
	cpuGeom.verts.push_back(glm::vec3(-1.f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// colours (these should be in linear space)
	// middle
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue

	// right
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue

	// left	
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue
	

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 9); // rightmost number means number of vertices
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
