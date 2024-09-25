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

std::vector<glm::vec3> colorPalette = {
    glm::vec3(0.5f, 0.5f, 0.5f), // Gray
    glm::vec3(0.0f, 0.0f, 1.0f), // Blue
    glm::vec3(0.5f, 0.0f, 0.0f), // Dark Red
    glm::vec3(0.0f, 1.0f, 0.0f), // Green
    glm::vec3(1.0f, 0.5f, 0.5f), // Light Red
    glm::vec3(0.0f, 1.0f, 1.0f), // Cyan
    glm::vec3(0.5f, 1.0f, 0.5f), // Light Green
};

// Ensure depth_n never exceeds the color palette size
glm::vec3 getColorForDepth(int depth_n) {
    return colorPalette[depth_n % colorPalette.size()];
}

CPU_Geometry sierpinski_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int depth_n, glm::vec3 color) {
    CPU_Geometry cpugeom;

    if (depth_n > 0) {
		glm::vec3 level_color = getColorForDepth(depth_n);

        glm::vec3 q1 = (p1 + p3) * 0.5f;
        glm::vec3 q2 = (p1 + p2) * 0.5f;
        glm::vec3 q3 = (p2 + p3) * 0.5f;

        // Draw black triangle at this level
        cpugeom.verts.push_back(q1);
        cpugeom.verts.push_back(q2);
        cpugeom.verts.push_back(q3);

        cpugeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
        cpugeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
        cpugeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));

		// subdivide triangles
        CPU_Geometry branch1 = sierpinski_triangle(p1, q2, q1, depth_n - 1, level_color);
        cpugeom.verts.insert(cpugeom.verts.end(), branch1.verts.begin(), branch1.verts.end());
        cpugeom.cols.insert(cpugeom.cols.end(), branch1.cols.begin(), branch1.cols.end());

        CPU_Geometry branch2 = sierpinski_triangle(q2, p2, q3, depth_n - 1, level_color);
        cpugeom.verts.insert(cpugeom.verts.end(), branch2.verts.begin(), branch2.verts.end());
        cpugeom.cols.insert(cpugeom.cols.end(), branch2.cols.begin(), branch2.cols.end());

        CPU_Geometry branch3 = sierpinski_triangle(q1, q3, p3, depth_n - 1, level_color);
        cpugeom.verts.insert(cpugeom.verts.end(), branch3.verts.begin(), branch3.verts.end());
        cpugeom.cols.insert(cpugeom.cols.end(), branch3.cols.begin(), branch3.cols.end());

    } else {
        // Base case: draw colored triangle using p1, p2, p3
        cpugeom.verts.push_back(p1);
        cpugeom.verts.push_back(p2);
        cpugeom.verts.push_back(p3);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
    }
    
    return cpugeom;
}

CPU_Geometry draw_square(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4){
	CPU_Geometry cpugeom;

	glm::vec3 color = glm::vec3(0.5f, 0.0f, 0.0f); // Dark Red

	// Define the two triangles that form the square (two triangles make a quad)
    // Triangle 1: p1, p2, p3
    cpugeom.verts.push_back(p1);
    cpugeom.verts.push_back(p2);
    cpugeom.verts.push_back(p3);

    // Triangle 2: p1, p3, p4
    cpugeom.verts.push_back(p1);
    cpugeom.verts.push_back(p3);
    cpugeom.verts.push_back(p4);

    // Set the same color for each vertex of the square (uniform color)
    for (int i = 0; i < 6; ++i) {
        cpugeom.cols.push_back(color); // Set color for each vertex
    }

    return cpugeom;
}


CPU_Geometry pythagoras_tree(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int depth_n){
	CPU_Geometry cpugeom;

	glm::vec3 color = getColorForDepth(depth_n);

	if (depth_n > 0){

		// draw trunk square
		cpugeom.verts.push_back(p1);
		cpugeom.verts.push_back(p2);
		cpugeom.verts.push_back(p3);

		cpugeom.verts.push_back(p1);
		cpugeom.verts.push_back(p3);
		cpugeom.verts.push_back(p4);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		
		// u: vector between p1 and p3 
		glm::vec3 u = (p3 - p1) * 0.5f;

		// v: vector u rotated ccw 90 degrees
		glm::vec3 v;
		v.x = -(u.y);
		v.y = u.x;
		v.z = 0.f;

		// z: point zenith of the triangle
		glm::vec3 z = p1 + v;

		// w: vector v rotated ccw 90 degrees
		glm::vec3 w;
		w.x = -(v.y);
		w.y = v.x;
		w.z = 0.f;

		// new points
		glm::vec3 q1 = p1 + w;
		glm::vec3 q2 = z + w;
		glm::vec3 q3 = p4 + v;
		glm::vec3 q4 = z + v;

		// draw the triangle
		
		cpugeom.verts.push_back(p1);
		cpugeom.verts.push_back(p4);
		cpugeom.verts.push_back(z);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);

		CPU_Geometry branch1 = pythagoras_tree(q1, p1, z, q2, depth_n -1);
		cpugeom.verts.insert(cpugeom.verts.end(), branch1.verts.begin(), branch1.verts.end());
        cpugeom.cols.insert(cpugeom.cols.end(), branch1.cols.begin(), branch1.cols.end());

		CPU_Geometry branch2 = pythagoras_tree(q4, z, p4, q3, depth_n - 1);
		cpugeom.verts.insert(cpugeom.verts.end(), branch2.verts.begin(), branch2.verts.end());
        cpugeom.cols.insert(cpugeom.cols.end(), branch2.cols.begin(), branch2.cols.end());
	}
	else{
		// Base Case: Draw branch square using points p1,p2,p3,p4
		cpugeom.verts.push_back(p1);
		cpugeom.verts.push_back(p2);
		cpugeom.verts.push_back(p3);

		cpugeom.verts.push_back(p1);
		cpugeom.verts.push_back(p3);
		cpugeom.verts.push_back(p4);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);

		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
		cpugeom.cols.push_back(color);
	}
	

	return cpugeom;
}

CPU_Geometry koch_snowflake(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int depth_n) {
    CPU_Geometry cpugeom;



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
	int curr_scene = -1;

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		std::vector<int> user_input = callbacks->getParameters();
		
		switch (user_input[1]){
			case 1:
				if (user_input[0] != curr_depth_n || user_input[1] != curr_scene){
					if (user_input[0] != curr_depth_n){
						curr_depth_n = user_input[0];
					}
				
					if (user_input[1] != curr_scene){
						curr_scene = user_input[1];

						cpuGeom.cols.clear();
						cpuGeom.verts.clear();

						gpuGeom.setCols(cpuGeom.cols);
						gpuGeom.setVerts(cpuGeom.verts);

						glEnable(GL_FRAMEBUFFER_SRGB);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
						window.swapBuffers();
					}
					cpuGeom = sierpinski_triangle(
						glm::vec3(-0.5f, -0.5f, 0.f), 
						glm::vec3(0.5f, -0.5f, 0.f), 
						glm::vec3(0.f, 0.5f, 0.f), 
						curr_depth_n,
						glm::vec3(0.5f,0.5f,0.5f)
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
				if (user_input[0] != curr_depth_n || user_input[1] != curr_scene){
					if (user_input[0] != curr_depth_n){
						curr_depth_n = user_input[0];
					}
				
					if (user_input[1] != curr_scene){
						curr_scene = user_input[1];

						cpuGeom.cols.clear();
						cpuGeom.verts.clear();

						gpuGeom.setCols(cpuGeom.cols);
						gpuGeom.setVerts(cpuGeom.verts);

						glEnable(GL_FRAMEBUFFER_SRGB);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
						window.swapBuffers();
					}

					cpuGeom = pythagoras_tree(
						glm::vec3(-0.1f, 0.1f, 0.f), 
						glm::vec3(-0.1f, -0.1f, 0.f), 
						glm::vec3(0.1f, -0.1f, 0.f),
						glm::vec3(0.1f, 0.1f, 0.f),
						curr_depth_n
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
			case 3:
				if (user_input[0] != curr_depth_n || user_input[1] != curr_scene){
					if (user_input[0] != curr_depth_n){
						curr_depth_n = user_input[0];
					}
				
					if (user_input[1] != curr_scene){
						curr_scene = user_input[1];

						cpuGeom.cols.clear();
						cpuGeom.verts.clear();

						gpuGeom.setCols(cpuGeom.cols);
						gpuGeom.setVerts(cpuGeom.verts);
					}
					cpuGeom = koch_snowflake(
						glm::vec3(-0.5f, -0.5f, 0.f), 
						glm::vec3(0.5f, -0.5f, 0.f),
						glm::vec3(0.f, 0.5f, 0.f), 
						curr_depth_n
					);

				}
				break;
			case 4:
				if (user_input[0] != curr_depth_n || user_input[1] != curr_scene){
					if (user_input[0] != curr_depth_n){
						curr_depth_n = user_input[0];
					}
				
					if (user_input[1] != curr_scene){
						curr_scene = user_input[1];

						cpuGeom.cols.clear();
						cpuGeom.verts.clear();

						gpuGeom.setCols(cpuGeom.cols);
						gpuGeom.setVerts(cpuGeom.verts);
					}
					cpuGeom = dragon_curve();
				}
				break;
		}
		
	}

	glfwTerminate();
	return 0;
}
