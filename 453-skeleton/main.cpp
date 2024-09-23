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
		
		if (action == GLFW_PRESS || action == GLFW_REPEAT){
			if (key == GLFW_KEY_R && action == GLFW_PRESS) {
				shader.recompile();
			}

			if (key == GLFW_KEY_UP) {
				depth_n += 1;
			}

			if (key == GLFW_KEY_DOWN) {
				if (depth_n > 0){
					depth_n -= 1;
				}
			}

			if (key == GLFW_KEY_RIGHT) {
				scene += 1;
			}

			if (key == GLFW_KEY_LEFT) {
				if (scene > 1){
					scene -= 1;
				}
			}
		}
	}

	std::vector<int> getParameters(){
		return std::vector<int>{depth_n, scene};
	}

	

private:
	ShaderProgram& shader;
	int depth_n = 0;
	int scene = 1;

};

CPU_Geometry sierpinski_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int depth_n, bool base_triangle){
	CPU_Geometry cpugeom;
	
	if (depth_n > 0){
		glm::vec3 q1((0.5 * p1.x) + (0.5 * p3.x), (0.5 * p1.y) + (0.5 * p3.y), 0.f);
		glm::vec3 q2((0.5 * p1.x) + (0.5 * p2.x), (0.5 * p1.y) + (0.5 * p2.y), 0.f);
		glm::vec3 q3((0.5 * p2.x) + (0.5 * p3.x), (0.5 * p2.y) + (0.5 * p3.y), 0.f);

		std::cout << "q1: (" << q1.x << ", " << q1.y << ")" << std::endl;
		std::cout << "q2: (" << q2.x << ", " << q2.y << ")" << std::endl;
		std::cout << "q3: (" << q3.x << ", " << q3.y << ")" << std::endl << std::endl;


		// draw black triangle
		cpugeom.verts.push_back(q1);
		cpugeom.verts.push_back(q2);
		cpugeom.verts.push_back(q3);

		cpugeom.cols.push_back(glm::vec3(0.f, 0.f,0.f));
		cpugeom.cols.push_back(glm::vec3(0.f, 0.f,0.f));
		cpugeom.cols.push_back(glm::vec3(0.f, 0.f,0.f));


		sierpinski_triangle(p1, q2, q3, depth_n - 1, false);
		sierpinski_triangle(q2, p2, q3, depth_n - 1, false);
		sierpinski_triangle(q1, q3, p3, depth_n - 1, false);


	}
	else if (depth_n == 0 && base_triangle){
		// draw colored triangle, add new points to cpugeom
		cpugeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
		cpugeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
		cpugeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

		cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
		cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 
		cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 

	}
	return cpugeom;
}

CPU_Geometry pythagoras_tree(){
	CPU_Geometry cpugeom;

	cpugeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 

	return cpugeom;
}

CPU_Geometry koch_snowflake(){
	CPU_Geometry cpugeom;
	
	cpugeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 

	return cpugeom;
}

CPU_Geometry dragon_curve(){
	CPU_Geometry cpugeom;
	
	cpugeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpugeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 
	cpugeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f)); 

	return cpugeom;
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

	int curr_depth_n = -1;

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		std::vector<int> user_input = callbacks->getParameters();
		
		switch (user_input[1]){
			case 1:
				if (user_input[0] != curr_depth_n){
					curr_depth_n = user_input[0];
					cpuGeom = sierpinski_triangle(
						glm::vec3(-0.5f, -0.5f, 0.f), 
						glm::vec3(0.5f, -0.5f, 0.f), 
						glm::vec3(0.f, 0.5f, 0.f), 
						user_input[0], 
						true
					);

					gpuGeom.setCols(cpuGeom.cols);
					gpuGeom.setVerts(cpuGeom.verts);

					shader.use();
					gpuGeom.bind();

					glEnable(GL_FRAMEBUFFER_SRGB);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glDrawArrays(GL_TRIANGLES, 0, GLsizei(cpuGeom.verts.size()));
					glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
					window.swapBuffers();
					
				}
				break;
			case 2:
				cpuGeom = pythagoras_tree();
				break;
			case 3:
				cpuGeom = koch_snowflake();
				break;
			case 4:
				cpuGeom = dragon_curve();
				break;
		}
		
	}

	glfwTerminate();
	return 0;
}
