#include <glad/glad.h>

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
#include "Panel.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// Location refers to the screen coordinates from -1 to 1, not the pixel numbers
struct UserParameters{
	glm::vec3 currMousePosition;
	glm::vec3 newMouseClickLocation;
	bool clicked = false;
};

class CurveEditorCallBack : public CallbackInterface {
public:
	CurveEditorCallBack() {}

	glm::vec3 normPixelPos(float screenPos_x, float screenPos_y){
		glm::vec4 cursor = {screenPos_x, screenPos_y, 0.f, 1.f};
		// std::cout << cursor.x << " " << cursor.y << std::endl;
		
		// translates by half a pixel for pixel center being considered for cursor position
		glm::mat4 pixel_centering_T = glm::translate(glm::mat4(1.f), glm::vec3(0.5f,0.5f,0.f));
		cursor = pixel_centering_T * cursor;

		// std::cout << cursor.x << " " << cursor.y << std::endl;
		
		// Scale coords down to 0-1
		glm::mat4 zero_to_one_S = glm::scale(glm::mat4(1.f), glm::vec3(0.00125f, 0.00125f, 0.f));
		cursor = zero_to_one_S * cursor;

		// std::cout << cursor.x << " " << cursor.y << std::endl;

		// Turn y from 0-1 to -1 to 0 (since xpos and ypos record position as positive downward)
		cursor.y = 1.f - cursor.y;

		// std::cout << cursor.x << " " << cursor.y << std::endl;

		// Scale then Translate values to make them between -1 to 1
		glm::mat4 normalize_S = glm::scale(glm::mat4(1.f), glm::vec3(2.f,2.f,0.f));
		glm::mat4 normalize_T = glm::translate(glm::mat4(1.f), glm::vec3(-1.f,-1.f,0.f));
		cursor = normalize_T * normalize_S * cursor;

		return glm::vec3(cursor.x, cursor.y, 0.f);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		Log::info("KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override {
		Log::info("MouseButtonCallback: button={}, action={}", button, action);

		// Left click adds a point
		// action = 1 is release of button
		if (button == 0 && action == 1){
			this->userParameters.clicked = true;
			this->userParameters.newMouseClickLocation = this->normPixelPos(this->userParameters.currMousePosition.x, this->userParameters.currMousePosition.y);
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) override {
		Log::info("CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
		this->userParameters.currMousePosition = glm::vec3(float(xpos), float(ypos), 0.f);

	}

	virtual void scrollCallback(double xoffset, double yoffset) override {
		Log::info("ScrollCallback: xoffset={}, yoffset={}", xoffset, yoffset);
	}

	virtual void windowSizeCallback(int width, int height) override {
		Log::info("WindowSizeCallback: width={}, height={}", width, height);
		CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
	}

	UserParameters getUserParameters(){
		UserParameters ret = this->userParameters;
		this->userParameters.clicked = false;
		return ret;
	}
private:
	UserParameters userParameters;
};

// Can swap the callback instead of maintaining a state machine
/*
class TurnTable3DViewerCallBack : public CallbackInterface {

public:
	TurnTable3DViewerCallBack() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {}
	virtual void cursorPosCallback(double xpos, double ypos) {}
	virtual void scrollCallback(double xoffset, double yoffset) {}
	virtual void windowSizeCallback(int width, int height) {

		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width, height);
	}
private:

};
*/

class CurveEditorPanelRenderer : public PanelRendererInterface {
public:
	CurveEditorPanelRenderer()
		: inputText(""), buttonClickCount(0), sliderValue(0.0f),
		dragValue(0.0f), inputValue(0.0f), checkboxValue(false),
		comboSelection(0)
	{
		// Initialize options for the combo box
		options[0] = "Option 1";
		options[1] = "Option 2";
		options[2] = "Option 3";

		// Initialize color (white by default)
		colorValue[0] = 1.0f; // R
		colorValue[1] = 1.0f; // G
		colorValue[2] = 1.0f; // B
	}

	virtual void render() override {
		// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector
		ImGui::Text("Selected Color: R: %.3f, G: %.3f, B: %.3f", colorValue[0], colorValue[1], colorValue[2]);

		// Text input
		ImGui::InputText("Input Text", inputText, IM_ARRAYSIZE(inputText));

		// Display the input text
		ImGui::Text("You entered: %s", inputText);

		// Button
		if (ImGui::Button("Click Me")) {
			buttonClickCount++;
		}
		ImGui::Text("Button clicked %d times", buttonClickCount);

		// Scrollable block
		ImGui::TextWrapped("Scrollable Block:");
		ImGui::BeginChild("ScrollableChild", ImVec2(0, 100), true); // Create a scrollable child
		for (int i = 0; i < 20; i++) {
			ImGui::Text("Item %d", i);
		}
		ImGui::EndChild();

		// Float slider
		ImGui::SliderFloat("Float Slider", &sliderValue, 0.0f, 100.0f, "Slider Value: %.3f");

		// Float drag
		ImGui::DragFloat("Float Drag", &dragValue, 0.1f, 0.0f, 100.0f, "Drag Value: %.3f");

		// Float input
		ImGui::InputFloat("Float Input", &inputValue, 0.1f, 1.0f, "Input Value: %.3f");

		// Checkbox
		ImGui::Checkbox("Enable Feature", &checkboxValue);
		ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		ImGui::Text("Selected: %s", options[comboSelection]);

		// Displaying current values
		ImGui::Text("Slider Value: %.3f", sliderValue);
		ImGui::Text("Drag Value: %.3f", dragValue);
		ImGui::Text("Input Value: %.3f", inputValue);
	}

	glm::vec3 getColor() const {
		return glm::vec3(colorValue[0], colorValue[1], colorValue[2]);
	}

private:
	float colorValue[3];  // Array for RGB color values
	char inputText[256];  // Buffer for input text
	int buttonClickCount; // Count button clicks
	float sliderValue;    // Value for float slider
	float dragValue;      // Value for drag input
	float inputValue;     // Value for float input
	bool checkboxValue;   // Value for checkbox
	int comboSelection;   // Index of selected option in combo box
	const char* options[3]; // Options for the combo box
};

glm::vec3 calculateBezierPoint(const std::vector<glm::vec3>& controlPoints, float u) {
    if (controlPoints.empty()){
		return glm::vec3();
	}
	std::vector<glm::vec3> temp = controlPoints;
    for (int j = 1; j < int(temp.size()); ++j) {
        for (int i = 0; i < int(temp.size()) - j; ++i) {
            temp[i] = (1.0f - u) * temp[i] + u * temp[i + 1];
        }
    }
    return temp[0];
}

std::vector<glm::vec3> generateQuadraticBSpline(const std::vector<glm::vec3>& controlPoints, int iterations) {
    if (controlPoints.size() < 2) {
        // Need at least 2 points for Chaikin's algorithm
        return controlPoints;
    }

    std::vector<glm::vec3> points = controlPoints;

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<glm::vec3> newPoints;

        // Start with the first control point (open curve)
        newPoints.push_back(points[0]);

        for (size_t i = 0; i < points.size() - 1; ++i) {
            glm::vec3 p = 0.75f * points[i] + 0.25f * points[i + 1];
            glm::vec3 q = 0.25f * points[i] + 0.75f * points[i + 1];
            newPoints.push_back(p);
            newPoints.push_back(q);
        }

        // End with the last control point (open curve)
        newPoints.push_back(points.back());

        // Update points for the next iteration
        points = newPoints;
    }

    return points;
}


int main() {
    Log::debug("Starting main");

    glfwInit();
    Window window(800, 800, "CPSC 453: Bézier Curve Example");
    Panel panel(window.getGLFWwindow());

    auto curve_editor_callback = std::make_shared<CurveEditorCallBack>();
    auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>();

    window.setCallbacks(curve_editor_callback);
    panel.setPanelRenderer(curve_editor_panel_renderer);

    ShaderProgram shader_program_default("shaders/test.vert", "shaders/test.frag");

    std::vector<glm::vec3> cp_positions_vector = {};
    glm::vec3 cp_point_colour = { 1.f, 0.f, 0.f }; // Red color for control points

    // Control points
    CPU_Geometry cp_point_cpu;
    GPU_Geometry cp_point_gpu;

    // Bézier curve points
    std::vector<glm::vec3> bezierCurvePoints;
    int segments = 100;

	// Bezier Curve
    CPU_Geometry bezier_cpu;
	GPU_Geometry bezier_gpu;

	// B-Spline curve points
	std::vector<glm::vec3> bSplinePoints = generateQuadraticBSpline(cp_positions_vector, 4); // 4 iterations

	// B-Spline curve
	CPU_Geometry bSpline_cpu;
	GPU_Geometry bSpline_gpu;

	int curr_scene = 1;

    while (!window.shouldClose()) {
		glfwPollEvents();
		glm::vec3 background_colour = curve_editor_panel_renderer->getColor();
		UserParameters changes = curve_editor_callback->getUserParameters();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_program_default.use();

		switch (curr_scene)
		{
		case 1:
			if (changes.clicked) {
				cp_positions_vector.push_back(changes.newMouseClickLocation);

				// Update control points in GPU
				cp_point_cpu.verts = cp_positions_vector;
				cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);

				cp_point_gpu.setVerts(cp_point_cpu.verts);
				cp_point_gpu.setCols(cp_point_cpu.cols);

				if (cp_positions_vector.size() >= 2) {
					

					// Regenerate Bézier curve points
					bezierCurvePoints.clear();
					for (int i = 0; i <= segments; ++i) {
						float t = i / (float)segments;
						bezierCurvePoints.push_back(calculateBezierPoint(cp_positions_vector, t));
					}

					// Update Bézier curve in GPU
					bezier_cpu.verts = bezierCurvePoints;
					bezier_cpu.cols = std::vector<glm::vec3>(bezier_cpu.verts.size(), glm::vec3(0, 0, 1)); // Blue color for Bézier curve

					bezier_gpu.setVerts(bezier_cpu.verts);
					bezier_gpu.setCols(bezier_cpu.cols);
				}
			}

			// Render control points
			cp_point_gpu.bind();
			glPointSize(15.f);
			glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

			// Render Bézier curve
			bezier_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, bezier_cpu.verts.size());
			break;

		case 2:
			if (changes.clicked) {
				cp_positions_vector.push_back(changes.newMouseClickLocation);

				// Update control points in GPU
					cp_point_cpu.verts = cp_positions_vector;
					cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);

					cp_point_gpu.setVerts(cp_point_cpu.verts);
					cp_point_gpu.setCols(cp_point_cpu.cols);

				if (cp_positions_vector.size() >= 3) {

					// Update B-Spline curve in GPU
					bSplinePoints = generateQuadraticBSpline(cp_positions_vector, 4);

					bSpline_cpu.verts = bSplinePoints;
					bSpline_cpu.cols = std::vector<glm::vec3>(bSpline_cpu.verts.size(), glm::vec3(0, 0, 1)); // Blue color for Bézier curve

					bSpline_gpu.setVerts(bSpline_cpu.verts);
					bSpline_gpu.setCols(bSpline_cpu.cols);
				}
			}

			// Render control points
			cp_point_gpu.bind();
			glPointSize(15.f);
			glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

			// Render B-Spline curve
			bSpline_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, bSpline_cpu.verts.size());
			break;

		default:
			break;
		}

		glDisable(GL_FRAMEBUFFER_SRGB);
		panel.render();
		window.swapBuffers();
	}


    glfwTerminate();
    return 0;
}