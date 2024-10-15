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

CPU_Geometry shipGeom(float scale, glm::vec3 center, glm::vec3 facing);

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
		transformationMatrix(1.0f)// This constructor sets it as the identity matrix
	{
		// Set default size
		cgeom.verts.push_back(glm::vec3(-0.09f, 0.09f, 0.f));
		cgeom.verts.push_back(glm::vec3(-0.09f, -0.09f, 0.f));
		cgeom.verts.push_back(glm::vec3(0.09f, -0.09f, 0.f));
		cgeom.verts.push_back(glm::vec3(-0.09f, 0.09f, 0.f));
		cgeom.verts.push_back(glm::vec3(0.09f, -0.09f, 0.f));
		cgeom.verts.push_back(glm::vec3(0.09f, 0.09f, 0.f));

		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 1.f));

	}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	glm::vec3 facing;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix;

	// Just testing to see if movement works
	void update(glm::vec3 displacement, float direction){
		CPU_Geometry newGeom;

		// if ship would move out of bounds
		bool goingOutOfBounds = false;
		if (!goingOutOfBounds){
			this->position += displacement;
		}

		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(direction), glm::vec3(0.f, 0.f, 1.f));
    	glm::vec3 newfacing = glm::vec3(rotationMatrix * glm::vec4(this->facing, 0.0f));

		this->facing = newfacing;

		std::cout << "Position: " << glm::to_string(this->position) << std::endl;
		std::cout << "Facing: " << glm::to_string(this->facing) << std::endl;


		newGeom = shipGeom(this->scale, this->position, this->facing);
		
		// add the newGeom verts to the GameObject by first deleting the old ones
		// no need to add the texture coords since they don't change

		this->cgeom.verts.clear();
		for (int i = 0; i < 6; i++){
			this->cgeom.verts.push_back(newGeom.verts[i]);
		}

		this->ggeom.setVerts(this->cgeom.verts);
		this->ggeom.setTexCoords(this->cgeom.texCoords);
	}
};

struct Parameters{
	glm::vec3 displacement = {0.f,0.f,0.f};
	float direction = 0.f;
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

			if (key == GLFW_KEY_S){
				playerInputs.displacement += glm::vec3(0.f, -0.016f, 0.f);
			}
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		glm::vec4 cursor = {xpos, ypos, 0.f, 1.f};

		// std::cout << cursor.x << " " << cursor.y << std::endl;
		
		// translates by half a pixel for pixel center being considered for cursor position
		glm::mat4 pixel_centering_T = glm::translate(glm::mat4(1.f), glm::vec3(0.5f,0.5f,0.f));
		cursor = pixel_centering_T * cursor;

		// std::cout << cursor.x << " " << cursor.y << std::endl;
		
		// Scale coords down to 0-1
		glm::mat4 zero_to_one_S = glm::scale(glm::mat4(1.f), glm::vec3(0.00125f, 0.00125f, 0.f));
		cursor = zero_to_one_S * cursor;

		std::cout << cursor.x << " " << cursor.y << std::endl;

		// Turn y from 0-1 to -1 to 0 (since xpos and ypos record position as positive downward)
		cursor.y = 1.f - cursor.y;

		// std::cout << cursor.x << " " << cursor.y << std::endl;

		// Scale then Translate values to make them between -1 to 1
		glm::mat4 normalize_S = glm::scale(glm::mat4(1.f), glm::vec3(2.f,2.f,0.f));
		glm::mat4 normalize_T = glm::translate(glm::mat4(1.f), glm::vec3(-1.f,-1.f,0.f));
		cursor = normalize_T * normalize_S * cursor;

		std::cout << cursor.x << " " << cursor.y << std::endl;
	}

	Parameters getParameters(){
		Parameters ret = playerInputs;
		playerInputs.direction = 0.f;
		playerInputs.displacement = glm::vec3(0.f,0.f,0.f);
		return ret;
	}

private:
	ShaderProgram& shader;
	Parameters playerInputs;
};

CPU_Geometry shipGeom(float scale, glm::vec3 center, glm::vec3 facing) {
	CPU_Geometry retGeom;

	// default width & height = 0.18f

	float halfWidth = 0.18f * 0.5f * scale; 
	float halfHeight = 0.18f * 0.5f * scale;

	// rotation angle wrt y-axis
	float angle =  acos(glm::dot(glm::vec3(0.f,1.f,0.f), facing));

	// vector from origin to the 4 corners of ship
	glm::vec3 v0 = glm::vec3(-halfWidth, halfHeight,0.f) - glm::vec3(0.f,0.f,0.f);
	glm::vec3 v1 = glm::vec3(-halfWidth, -halfHeight,0.f) - glm::vec3(0.f,0.f,0.f);
	glm::vec3 v2 = glm::vec3(halfWidth, -halfHeight,0.f) - glm::vec3(0.f,0.f,0.f);
	glm::vec3 v3 = glm::vec3(halfWidth, halfHeight,0.f) - glm::vec3(0.f,0.f,0.f);

	// rotate the vectors by the rotation angle
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 0.f, 1.f));

	glm::vec3 v0prime = glm::vec3(rotationMatrix * glm::vec4(v0, 0.0f));
	glm::vec3 v1prime = glm::vec3(rotationMatrix * glm::vec4(v1, 0.0f));
	glm::vec3 v2prime = glm::vec3(rotationMatrix * glm::vec4(v2, 0.0f));
	glm::vec3 v3prime = glm::vec3(rotationMatrix * glm::vec4(v3, 0.0f));

	// get the point of the corners of the ship
	glm::vec3 p0 = center + v0prime;
	glm::vec3 p1 = center + v1prime;
	glm::vec3 p2 = center + v2prime;
	glm::vec3 p3 = center + v3prime;

	retGeom.verts.push_back(p0);
	retGeom.verts.push_back(p1);
	retGeom.verts.push_back(p2);
	retGeom.verts.push_back(p0);
	retGeom.verts.push_back(p2);
	retGeom.verts.push_back(p3);
	
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

	std::cout << "Vertices:" << std::endl;
	std::cout << glm::to_string(retGeom.verts[0]) << std::endl;
	std::cout << glm::to_string(retGeom.verts[1]) << std::endl;
	std::cout << glm::to_string(retGeom.verts[2]) << std::endl;
	std::cout << glm::to_string(retGeom.verts[3]) << std::endl;
	std::cout << glm::to_string(retGeom.verts[4]) << std::endl;
	std::cout << glm::to_string(retGeom.verts[5]) << std::endl;

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
		float direction_change = user_input.direction;
		glm::vec3 displacement_change = user_input.displacement;


		// Recalculate ship position
		if (direction_change != 0.f || !(displacement_change[0] == 0.f && displacement_change[1] == 0.f && displacement_change[2] == 0.f)){
			ship.update(displacement_change, direction_change);
		}

		ship.ggeom.setVerts(ship.cgeom.verts);
		ship.ggeom.setTexCoords(ship.cgeom.texCoords);
		diamond.ggeom.setVerts(diamond.cgeom.verts);
		diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

		// First render the ship
		ship.ggeom.bind();
		ship.texture.bind();

		// Here go the transformations
		/*
		
		*/
		
		
		// Draw ship then unbind texture
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		ship.texture.unbind();

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
