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

struct Parameters{
	glm::vec3 displacement = {0.f,0.f,0.f};
	glm::vec3 cursorPos = {0.f,0.f,0.f};
};

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Ship Constructor
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		facing(0.f, 1.f, 0.f)
	{
		
		cgeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));

		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 1.f));

		S_Matrix = glm::scale(glm::mat4(1.f), glm::vec3(default_ship_size, default_ship_size, 0.f));
		T_Matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f,0.f,0.f));
		R_matrix = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.f,0.f,1.f));

	}

	// Diamond Constructor
	// FIXME: pos must be set to the T_Matrix
	GameObject(std::string texturePath, GLenum textureInterpolation, glm::vec3 pos, glm::vec3 face) :
		texture(texturePath, textureInterpolation),
		position(pos),
		facing(face)
	{
		
		cgeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		cgeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));

		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		cgeom.texCoords.push_back(glm::vec2(1.f, 1.f));

		S_Matrix = glm::scale(glm::mat4(1.f), glm::vec3(default_diamond_size, default_diamond_size, 0.f));
		T_Matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f,0.f,0.f));
		R_matrix = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.f,0.f,1.f));
	}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	float default_ship_size = 0.18f;
	float default_diamond_size = 0.18f;

	glm::vec3 position;
	glm::vec3 facing;
	glm::mat4 S_Matrix;
	glm::mat4 R_matrix;
	glm::mat4 T_Matrix;
	

	glm::mat4 getTransformationMatrix(){
		return T_Matrix * R_matrix * S_Matrix;
	}

	void updateShip(Parameters user_input, int counter){
		// Compute changes
		glm::vec3 mustFace = user_input.cursorPos - this->position;
		glm::vec3 face_uvector = mustFace / float(mustFace.length());

		// Compute the change in position in the direction of the ship's current facing vector
		glm::vec3 displacement = user_input.displacement.y * this->facing; // Move in the facing direction

		// Update position
		this->position += displacement;

		float angle = acos(glm::dot(this->facing, face_uvector));

		glm::vec3 posChange = this->position + (face_uvector * user_input.displacement);


		// Apply changes to ship object data members
		this->facing = face_uvector;

		// Change the transformation matrices
		glm::mat4 added_S = glm::scale(glm::mat4(1.f), glm::vec3(1.f + counter * 0.25, 1.f + counter * 0.25, 1.f));
		this->S_Matrix = added_S * this->S_Matrix;

		glm::mat4 added_R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f,0.f,1.f));
		this->R_matrix = added_R * this->R_matrix;

		glm::mat4 added_T = glm::translate(glm::mat4(1.0f), posChange);
		this->T_Matrix = added_T * this->T_Matrix;
		
	}

	void updateDiamondPosition(){
		// check if going out of bounds
		if (abs((this->position + this->facing).x) >= 1.0f || abs((this->position + this->facing).y) >= 1.0f){
			this->facing = -this->facing;
		}
		glm::vec3 displacement = this->facing * 0.015f;
		this->position += displacement;

		glm::mat4 added_T = glm::translate(glm::mat4(1.f), displacement);
		this->T_Matrix = added_T * this->T_Matrix;
	}

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

		playerInputs.cursorPos.x = cursor.x;
		playerInputs.cursorPos.y = cursor.y;
	}

	Parameters getParameters(){
		return playerInputs;
	}

private:
	ShaderProgram& shader;
	Parameters playerInputs;
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
	std::shared_ptr<MyCallbacks> callback = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callback); // can also update callbacks to new ones

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
	GameObject diamond1("textures/diamond.png", GL_NEAREST, glm::vec3(0.0f,0.f,0.f), glm::vec3(0.5f,0.25f,0.f));
	// GameObject diamond2("textures/diamond.png", GL_NEAREST, glm::vec3(0.0f,0.f,0.f), glm::vec3(0.5f,0.5f,0.f));
	// GameObject diamond3("textures/diamond.png", GL_NEAREST, glm::vec3(0.0f,0.f,0.f), glm::vec3(0.5f,0.5f,0.f));
	// GameObject diamond4("textures/diamond.png", GL_NEAREST, glm::vec3(0.0f,0.f,0.f), glm::vec3(0.5f,0.5f,0.f));
	// GameObject diamond5("textures/diamond.png", GL_NEAREST, glm::vec3(0.0f,0.f,0.f), glm::vec3(0.5f,0.5f,0.f));


	// std::vector<GameObject> uncollectedDiamonds = std::vector<GameObject>();
	// uncollectedDiamonds.push_back(diamond1);
	// uncollectedDiamonds.push_back(diamond2);
	// uncollectedDiamonds.push_back(diamond3);
	// uncollectedDiamonds.push_back(diamond4);
	// uncollectedDiamonds.push_back(diamond5);

	int score = 0;
	bool start = false;

	// RENDER LOOP
	while (!window.shouldClose()) {
		
		Parameters user_input = callback->getParameters();

		glfwPollEvents();

		shader.use();

		// Clear the screen
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ship.updateShip(user_input, 0);
		// // Get the user input details
		// glm::vec2 cursorPos = user_input.cursorPos;
		// glm::vec3 displacement_change = user_input.displacement;

		// // Recalculate ship position (this is erroneous because it does not consider rotation #FIXME)
		// ship.T_Matrix = glm::translate(glm::mat4(1.f), displacement_change);
		// ship.position = displacement_change;
		
		ship.ggeom.setVerts(ship.cgeom.verts);
		ship.ggeom.setTexCoords(ship.cgeom.texCoords);
		diamond1.ggeom.setVerts(diamond1.cgeom.verts);
		diamond1.ggeom.setTexCoords(diamond1.cgeom.texCoords);

		// First render the ship
		ship.ggeom.bind();
		ship.texture.bind();

		// Here go the transformations
		glm::mat4 M_ship = ship.getTransformationMatrix();
		glUniformMatrix4fv(0,1,GL_FALSE, glm::value_ptr(M_ship));
		
		// Draw ship then unbind texture
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		ship.texture.unbind();

		// Then render the diamonds
		diamond1.ggeom.bind();
		diamond1.texture.bind();

		diamond1.updateDiamondPosition();

		// Here go the transformations
		glm::mat4 M_diamond = diamond1.getTransformationMatrix();
		glUniformMatrix4fv(0,1,GL_FALSE, glm::value_ptr(M_diamond));

		

		// // Draw diamond then unbind texture
		glDrawArrays(GL_TRIANGLES, 0, 6);
		diamond1.texture.unbind();
		
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

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
		ImGui::Text("Score: %d", score); // Second parameter gets passed into "%d"

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
