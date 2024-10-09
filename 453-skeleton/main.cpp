#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

CPU_Geometry shipGeom(float width, float height, glm::vec3 offset, float angle);

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		facing(0.f, 1.f, 0.f),
		scale(1),
		transformationMatrix(1.0f) // This constructor sets it as the identity matrix
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	glm::vec3 facing;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix;

	// Just testing to see if movement works
	CPU_Geometry update(glm::vec3 displacement, glm::vec3 direction){
		this->position = displacement;
		glm::vec3 newFacing = (this->facing + direction) / glm::length(this->facing + direction);

		float angle = glm::dot(this->facing, newFacing) / (glm::length(this->facing) * glm::length(newFacing));

		return shipGeom(0.18f, 0.18f, this->position, angle);
	}
};

struct Parameters{
	glm::vec3 displacement = {0.f,0.f,0.f};
	glm::vec3 direction = {0.f,0.f,0.f};
};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}

		if (action == GLFW_PRESS || action == GLFW_REPEAT){
			if (key == GLFW_KEY_W){
				playerInputs.displacement += glm::vec3(0.f, 0.016f, 0.f);
			}

			if (key == GLFW_KEY_A){
				playerInputs.direction += glm::vec3(-1.f, 0.f, 0.f);
			}

			if (key == GLFW_KEY_S){
				playerInputs.displacement += glm::vec3(0.f, -.016f, 0.f);
			}

			if (key == GLFW_KEY_D){
				playerInputs.direction += glm::vec3(1.f, 0.f, 0.f);
			}
		}
	}
	// assume fixed window size
	virtual void cursorPosCallback(double xpos, double ypos){
		glm::vec4 cursor = {xpos, ypos, 0.f, 1.f};
		std::cout << cursor.x << " " << cursor.y << std::endl;

		// translates by half a pixel for pixel center being considered for cursor position
		glm::mat4 pixel_centering_T = glm::translate(glm::mat4(1.f), glm::vec3(0.5f,0.5f,0.f));

		// normalize: x will be between 0 and 2 and y will be between -2 and 0
		// 0 and 2 because our square goes from -1 to 1
		// glm::mat4 normalize_S = glm::scale(glm::mat4(1.f), );

		// translates by 1 to the left to make it between -1 and 1
		


	}

	Parameters getParameters(){
		Parameters ret = playerInputs;

		return ret;
	}

private:
	ShaderProgram& shader;
	Parameters playerInputs;
};

CPU_Geometry shipGeom(float width, float height, glm::vec3 offset, float angle) {
	CPU_Geometry retGeom;

	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	retGeom.verts.push_back(glm::vec3(-halfWidth + offset.x, halfHeight + offset.y, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth + offset.x, -halfHeight + offset.y, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + offset.x, -halfHeight + offset.y, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth + offset.x, halfHeight + offset.y, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + offset.x, -halfHeight + offset.y, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + offset.x, halfHeight + offset.y, 0.f));

	// This rotates them based on angle
	for (size_t i = 0; i < retGeom.verts.size(); i++){
		float x = retGeom.verts[i].x;
		float y = retGeom.verts[i].y;

		// Apply the 2D rotation formula
        float newX = x * cos(angle) - y * sin(angle);
        float newY = x * sin(angle) + y * cos(angle);

        // Update the vertex position
        retGeom.verts[i].x = newX;
        retGeom.verts[i].y = newY;
	}
	// vertices for the spaceship quad
	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.
	
	// retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	// retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	// retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	// retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	// retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	// retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));
	

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

CPU_Geometry diamondGeom(){
	CPU_Geometry retGeom;

	// Square is made of two triangles, so 6 vertices
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));


	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));

	return retGeom;
}

// END EXAMPLES

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	std::shared_ptr<MyCallbacks> callback = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callback); // can also update callbacks to new ones

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
	GameObject diamond("textures/diamond.png", GL_NEAREST);

	ship.cgeom = shipGeom(0.18f, 0.18f, glm::vec3(0.f,0.f,0.f), 0.f);

	// translation
	glm::mat4 M = glm::translate(glm::mat4(1.f), glm::vec3(0.1f, 0.1f, 0.f));

	for(glm::vec3& vert : ship.cgeom.verts){
		vert = M * glm::vec4(vert, 1.f);
	}

	diamond.cgeom = diamondGeom();


	ship.ggeom.setVerts(ship.cgeom.verts);
	ship.ggeom.setTexCoords(ship.cgeom.texCoords);
	diamond.ggeom.setVerts(diamond.cgeom.verts);
	diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score;
		Parameters user_input = callback->getParameters();

		glfwPollEvents();

		shader.use();

		// Clear the screen
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get the user input details
		// glm::vec3 direction_change = user_input.direction;
		// glm::vec3 displacement_change = user_input.displacement;

		// Recalculate ship position
		// ship.cgeom = ship.update(displacement_change, direction_change);

		ship.ggeom.setVerts(ship.cgeom.verts);
		ship.ggeom.setTexCoords(ship.cgeom.texCoords);
		diamond.ggeom.setVerts(diamond.cgeom.verts);
		diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

		// First render the ship
		ship.ggeom.bind();
		ship.texture.bind();

		// Here go the transformations
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(M));
		
		
		// Draw ship then unbind texture
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// ship.texture.unbind();

		// // Then render the diamond
		// diamond.ggeom.bind();
		// diamond.texture.bind();

		// // Here go the transformations
		// /*
		
		// */

		// // Draw diamond then unbind texture
		// glDrawArrays(GL_TRIANGLES, 0, 6);
		// diamond.texture.unbind();
		
		// glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("Score: %d", 0); // Second parameter gets passed into "%d"

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
