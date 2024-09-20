#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

struct Parameters {
	const float tStep = 0.1f;
	const float uStep = 0.1f;

	float t = 1.0f;
	float u = 0.0f;

	bool isDifferent(Parameters p){
		return p.t != t || p.u != u;
	}
};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		// if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		// 	shader.recompile();
		// }
		
		if (action == GLFW_PRESS || action == GLFW_REPEAT){
			if (key == GLFW_KEY_R && action == GLFW_PRESS) {
				shader.recompile();
			}

			if (key == GLFW_KEY_UP) {
				parameters.t += parameters.tStep;
				std::cout << "t:" << parameters.t << std::endl;
			}

			if (key == GLFW_KEY_DOWN) {
				parameters.t -= parameters.tStep;
				std::cout << "t:" << parameters.t << std::endl;
			}

			if (key == GLFW_KEY_RIGHT) {
				parameters.u += parameters.uStep;
				std::cout << "u:" << parameters.u << std::endl;
			}

			if (key == GLFW_KEY_LEFT) {
				parameters.u -= parameters.uStep;
				std::cout << "u:" << parameters.u << std::endl;
			}
		}
	}

	Parameters getParameters(){
		return parameters;
	}

private:
	ShaderProgram& shader;
	Parameters parameters;
};

// class MyCallbacks2 : public CallbackInterface {

// public:
// 	MyCallbacks2() {}

// 	virtual void keyCallback(int key, int scancode, int action, int mods) {
// 		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
// 			std::cout << "called back" << std::endl;
// 		}
// 	}
// };
// END EXAMPLES

CPU_Geometry generateSin(Parameters p){
	CPU_Geometry cpuGeom;
	for (float x = -1.0f; x <= 1.0f; x+=0.01f){
		cpuGeom.verts.push_back(glm::vec3(x, p.u * sin(x * p.t), 0.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	}
	return cpuGeom;
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	std::shared_ptr<MyCallbacks> callbacks = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;
	
	Parameters p;
	cpuGeom = generateSin(p);
	

	// // vertices
	// cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	// cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	// cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// // colours (these should be in linear space)
	// cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
	// cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
	// cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		Parameters newP = callbacks->getParameters();

		if (newP.isDifferent(p)){
			cpuGeom = generateSin(newP);
			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
		}

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(cpuGeom.verts.size()));
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
