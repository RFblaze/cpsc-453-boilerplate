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

class CurveEditorCallBack : public CallbackInterface {
public:
	CurveEditorCallBack() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		Log::info("KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override {
		Log::info("MouseButtonCallback: button={}, action={}", button, action);
	}

	virtual void cursorPosCallback(double xpos, double ypos) override {
		Log::info("CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
	}

	virtual void scrollCallback(double xoffset, double yoffset) override {
		Log::info("ScrollCallback: xoffset={}, yoffset={}", xoffset, yoffset);
	}

	virtual void windowSizeCallback(int width, int height) override {
		Log::info("WindowSizeCallback: width={}, height={}", width, height);
		CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
	}
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

glm::vec3 calculateBezierPoint(const std::vector<glm::vec3>& controlPoints, float t) {
    std::vector<glm::vec3> temp = controlPoints;
    for (int j = 1; j < int(temp.size()); ++j) {
        for (int i = 0; i < int(temp.size()) - j; ++i) {
            temp[i] = (1.0f - t) * temp[i] + t * temp[i + 1];
        }
    }
    return temp[0];
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

    std::vector<glm::vec3> cp_positions_vector = {
        {-.5f, -.5f, 0.f},
        { .5f, -.5f, 0.f},
        { .5f,  .5f, 0.f},
        {-.5f,  .5f, 0.f}
    };
    glm::vec3 cp_point_colour = { 1.f, 0.f, 0.f }; // Red color for control points

    // Set up control points in GPU
    CPU_Geometry cp_point_cpu;
    cp_point_cpu.verts = cp_positions_vector;
    cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
    GPU_Geometry cp_point_gpu;
    cp_point_gpu.setVerts(cp_point_cpu.verts);
    cp_point_gpu.setCols(cp_point_cpu.cols);

    // Generate Bézier curve points
    std::vector<glm::vec3> bezierCurvePoints;
    int segments = 100;
    for (int i = 0; i <= segments; ++i) {
        float t = i / (float)segments;
        bezierCurvePoints.push_back(calculateBezierPoint(cp_positions_vector, t));
    }

    CPU_Geometry bezier_cpu;
    bezier_cpu.verts = bezierCurvePoints;
    bezier_cpu.cols = std::vector<glm::vec3>(bezier_cpu.verts.size(), glm::vec3(0, 0, 1)); // Blue color for Bézier curve
    GPU_Geometry bezier_gpu;
    bezier_gpu.setVerts(bezier_cpu.verts);
    bezier_gpu.setCols(bezier_cpu.cols);

    while (!window.shouldClose()) {
        glfwPollEvents();
        glm::vec3 background_colour = curve_editor_panel_renderer->getColor();

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_program_default.use();

        // Render control points
        cp_point_gpu.bind();
        glPointSize(15.f);
        glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

        // Render Bézier curve
        bezier_gpu.bind();
        glDrawArrays(GL_LINE_STRIP, 0, bezier_cpu.verts.size());

        glDisable(GL_FRAMEBUFFER_SRGB);
        panel.render();
        window.swapBuffers();
    }

    glfwTerminate();
    return 0;
}