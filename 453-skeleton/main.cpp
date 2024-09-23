#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

struct TriangleData {
	float increment = 0.1f;

	glm::vec3 point1 = glm::vec3(-0.5f, -0.5f, 0.f);
	glm::vec3 point2 = glm::vec3(0.5f, -0.5f, 0.f);
	glm::vec3 point3 = glm::vec3(0.f, 0.5f, 0.f);

	bool isDifferent(TriangleData newData){
		return point1 != newData.point1 ||
		point2 != newData.point2 ||
		point3 != newData.point3;
	}

};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader){}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS){
			
			if (key == GLFW_KEY_R ) {
				shader.recompile();
			}

			if (key == GLFW_KEY_UP){
				std::cout << "press up" << std::endl;
				triangleData.point1.y += triangleData.increment;
				triangleData.point2.y += triangleData.increment;
				triangleData.point3.y += triangleData.increment;
			}

			if (key == GLFW_KEY_DOWN){
				std::cout << "press down" << std::endl;
				triangleData.point1.y -= triangleData.increment;
				triangleData.point2.y -= triangleData.increment;
				triangleData.point3.y -= triangleData.increment;
			}

			if (key == GLFW_KEY_LEFT){
				std::cout << "press left" << std::endl;
				triangleData.point1.x -= triangleData.increment;
				triangleData.point2.x -= triangleData.increment;
				triangleData.point3.x -= triangleData.increment;
			}

			if (key == GLFW_KEY_RIGHT){
				std::cout << "press right" << std::endl;
				triangleData.point1.x += triangleData.increment;
				triangleData.point2.x += triangleData.increment;
				triangleData.point3.x += triangleData.increment;
			}
		}	
			
	}

	TriangleData getTriangleData(){
		return triangleData;
	}

private:
	ShaderProgram& shader;
	TriangleData triangleData;
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
	std::shared_ptr<MyCallbacks> callbacks = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	// vertices
	// middle
	
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// colours (these should be in linear space)
	// middle
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue
	

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	TriangleData currTriangle; 

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		TriangleData newTriangle = callbacks->getTriangleData();
		if (currTriangle.isDifferent(newTriangle)){

			cpuGeom.verts.push_back(newTriangle.point1);
			cpuGeom.verts.push_back(newTriangle.point2);
			cpuGeom.verts.push_back(newTriangle.point3);

			cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); // red
			cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f)); // green
			cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f)); // blue

			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);

		}

		currTriangle = newTriangle;


		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(cpuGeom.verts.size())); // rightmost number means number of vertices
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
