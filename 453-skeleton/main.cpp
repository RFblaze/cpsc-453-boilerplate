#include <glad/glad.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <cmath>

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
	// Tracking
	glm::vec3 currMousePosition;
	glm::vec3 newMouseClickLocation;

	// User-Inputs
	bool clicked = false;
	int buttonPressedASCII = 80;
	bool isWireframe = true;
	bool cameraEnabled = false;

	// for the move function
	glm::vec3 firstClick;
	glm::vec3 clickRelease;

	
	
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
		this->userParameters.buttonPressedASCII = key;

		// if spacebar is clicked, toggle camera mode
		if (key == 32){
			this->userParameters.cameraEnabled = !this->userParameters.cameraEnabled;
		}
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
		this->userParameters.buttonPressedASCII = NULL;
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

glm::vec3 updateCameraPosition(glm::vec3 cameraTarget, glm::vec3 cameraUp,
	float zoom,
	float pitch,
	float yaw) {
    // Convert spherical coordinates to Cartesian coordinates
    float x = zoom * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    float y = zoom * sin(glm::radians(pitch));
    float z = zoom * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    glm::vec3 cameraPos = glm::vec3(x, y, z) + cameraTarget; // Offset position relative to the target
	return cameraPos;
}


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

std::vector<glm::vec3> createSurfaceOfRevolution(const std::vector<glm::vec3>& bSplineCurvePoints, int numSlices) {
    std::vector<glm::vec3> surfaceVertices;
    float angleStep = 360.0f / numSlices;

    for (const glm::vec3& point : bSplineCurvePoints) {
        for (int slice = 0; slice < numSlices; ++slice) {
            float angle = glm::radians(slice * angleStep);
            float x = point.x * cos(angle) - point.z * sin(angle);
            float z = point.x * sin(angle) + point.z * cos(angle);
            surfaceVertices.emplace_back(x, point.y, z);
        }
    }
    return surfaceVertices;
}

// Define the B-spline basis function
float BSplineBasis(int i, int k, float t, const std::vector<float>& knots) {
    if (k == 1) {
        return (t >= knots[i] && t < knots[i + 1]) ? 1.0f : 0.0f;
    }

    float denom1 = knots[i + k - 1] - knots[i];
    float denom2 = knots[i + k] - knots[i + 1];
    float term1 = (denom1 != 0.0f) ? ((t - knots[i]) / denom1) * BSplineBasis(i, k - 1, t, knots) : 0.0f;
    float term2 = (denom2 != 0.0f) ? ((knots[i + k] - t) / denom2) * BSplineBasis(i + 1, k - 1, t, knots) : 0.0f;

    return term1 + term2;
}

// Generate the tensor product surface
std::vector<std::vector<glm::vec3>> generateTensorSurface(std::vector<std::vector<glm::vec3>> controlPoints, int resolutionU, int resolutionV) {
    std::vector<std::vector<glm::vec3>> surface(resolutionU, std::vector<glm::vec3>(resolutionV));

    // Knot vectors for uniform B-splines
    std::vector<float> knotsU = { 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 2.0f };
    std::vector<float> knotsV = { 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 2.0f };

    float stepU = 1.0f / (resolutionU - 1);
    float stepV = 1.0f / (resolutionV - 1);

    for (int uIndex = 0; uIndex < resolutionU; ++uIndex) {
        for (int vIndex = 0; vIndex < resolutionV; ++vIndex) {
            float u = uIndex * stepU * 2.0f; // Map to [0, 2] range for knots
            float v = vIndex * stepV * 2.0f; // Map to [0, 2] range for knots

            glm::vec3 point(0.0f);

            // Accumulate contributions from each control point
            for (size_t i = 0; i < controlPoints.size(); ++i) {
                for (size_t j = 0; j < controlPoints[i].size(); ++j) {
                    float basisU = BSplineBasis(i, 3, u, knotsU);
                    float basisV = BSplineBasis(j, 3, v, knotsV);
                    point += basisU * basisV * controlPoints[i][j];
                }
            }

            surface[uIndex][vIndex] = point;
        }
    }

    return surface;
}

void makeTensorVertices(const std::vector<std::vector<glm::vec3>>& tensorSurface, std::vector<glm::vec3>& flattenedVertices) {
    int rows = tensorSurface.size();
    int cols = tensorSurface[0].size();

    // Generate vertices for rendering using GL_TRIANGLES
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            // Define two triangles for each grid cell
            glm::vec3 v0 = tensorSurface[i][j];
            glm::vec3 v1 = tensorSurface[i][j + 1];
            glm::vec3 v2 = tensorSurface[i + 1][j];
            glm::vec3 v3 = tensorSurface[i + 1][j + 1];

            // Triangle 1
            flattenedVertices.push_back(v0);
            flattenedVertices.push_back(v1);
            flattenedVertices.push_back(v2);

            // Triangle 2
            flattenedVertices.push_back(v2);
            flattenedVertices.push_back(v1);
            flattenedVertices.push_back(v3);
        }
    }
}


int main() {
    Log::debug("Starting main");

    glfwInit();
    Window window(800, 800, "CPSC 453: Assignment 3");
    Panel panel(window.getGLFWwindow());

    auto curve_editor_callback = std::make_shared<CurveEditorCallBack>();
    auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>();

    window.setCallbacks(curve_editor_callback);
    panel.setPanelRenderer(curve_editor_panel_renderer);

    ShaderProgram shader_program_default("shaders/test.vert", "shaders/test.frag");

	// Camera
	glm::vec3 cameraPos = {0.f,0.f,3.f};      // Camera position
	glm::vec3 cameraTarget = {0.f,0.f,0.f};   // Point the camera looks at (in this case, the origin glm::vec3(0.f,0.f,0.f))
	glm::vec3 cameraUp = {0.f,1.f,0.f};       // "Up" direction for the camera (default glm::vec3(0.0f, 1.0f, 0.0f))

	glm::mat4 defaultView = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	glm::mat4 defaultProjection = glm::perspective(glm::radians(45.0f), 1.f, 0.1f, 100.0f);


	float zoom = 10.0f;       // Distance from the target (controls zoom)
	float pitch = 0.0f;       // Rotation around the X-axis (up/down movement)
	float yaw = 0.0f;         // Rotation around the Y-axis (left/right movement)

    // Control points
	std::vector<glm::vec3> cp_positions_vector = {};
    glm::vec3 cp_point_colour = { 1.f, 0.f, 0.f }; // Red color for control points

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


	// Surface of Revolution
	std::vector<glm::vec3> surfaceVertices;

	CPU_Geometry surface_cpu;
	GPU_Geometry surface_gpu;

	/*
	Tensor product surface calculation goes here
	*/
	std::vector< std::vector<glm::vec3> > controlPointsTensor1 = {
		{glm::vec3(-2.f,0.f,-2.f), glm::vec3(-1.f,0.f,-2.f), glm::vec3(0.f,0.f,-2.f), glm::vec3(1.f,0.f,-2.f), glm::vec3(2.f,0.f,-2.f)},
		{glm::vec3(-2.f,0.f,-1.f), glm::vec3(-1.f,1.f,-1.f), glm::vec3(0.f,1.f,-1.f), glm::vec3(1.f,1.f,-1.f), glm::vec3(2.f,0.f,-1.f)},
		{glm::vec3(-2.f,0.f,0.f), glm::vec3(-1.f,1.f,0.f), glm::vec3(0.f,-1.f,0.f), glm::vec3(1.f,1.f,0.f), glm::vec3(2.f,0.f,0.f)},
		{glm::vec3(-2.f,0.f,1.f), glm::vec3(-1.f,1.f,1.f), glm::vec3(0.f,1.f,1.f), glm::vec3(1.f,1.f,1.f), glm::vec3(2.f,0.f,1.f)},
		{glm::vec3(-2.f,0.f,-2.f), glm::vec3(-1.f,0.f,2.f), glm::vec3(0.f,0.f,2.f), glm::vec3(1.f,0.f,2.f), glm::vec3(2.f,0.f,2.f)}
	};

	std::vector<std::vector<glm::vec3>> controlPointsTensor2 = {
		{ { -1.5f, -1.0f, 0.0f }, { -0.5f, -1.0f, 1.2f }, {  0.5f, -1.0f, 0.8f }, {  1.5f, -1.0f, 0.0f } },
		{ { -1.5f,  0.0f, 1.5f }, { -0.5f,  0.0f, 2.0f }, {  0.5f,  0.0f, 1.5f }, {  1.5f,  0.0f, 0.5f } },
		{ { -1.5f,  1.0f, 0.0f }, { -0.5f,  1.0f, 0.5f }, {  0.5f,  1.0f, 0.2f }, {  1.5f,  1.0f, 0.0f } }
	};

	std::vector<std::vector<glm::vec3>> TensorProductSurface1 = generateTensorSurface(controlPointsTensor1, 20, 20);
	std::vector<std::vector<glm::vec3>> TensorProductSurface2 = generateTensorSurface(controlPointsTensor2, 20, 20);

	std::vector<glm::vec3> tensorVertices;

	CPU_Geometry tensor_cpu;
	GPU_Geometry tensor_gpu;

	int curr_scene = 1;

	// Place by default
	int mode = 80;

	std::vector<glm::vec3> test;
	// This is a control environment for testing
			
	test.push_back(glm::vec3(0.0f,-1.0f,0.0f));
	test.push_back(glm::vec3(-0.24f,-0.88f,0.0f));
	test.push_back(glm::vec3(-0.36f,-0.74f,0.0f));
	test.push_back(glm::vec3(-0.22f,-0.69f,0.0f));
	test.push_back(glm::vec3(-0.13f,-0.47f,0.0f));

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

		

		// if P is clicked, the user places control points
		// if M is clicked, the user moves control points with click-and-drag
		// if D is clicked, the user deletes specific control points
		// if R is clicked, it deletes all control points & curve
		// if Spacebar is clicked, it toggles 2D/3D camera mode
		mode = changes.buttonPressedASCII;

		// if W is clicked, the display toggles between wireframe and solid
		if (mode == 87){
			changes.isWireframe = !changes.isWireframe;
		}

		// if Spacebar is clicked, it toggles 2D/3D camera mode
		if (changes.cameraEnabled){
			cameraPos = updateCameraPosition(cameraTarget, cameraUp, zoom, pitch, yaw);

			glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 800.f, 0.1f, 100.f);

			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));
		}
		// 	else{
		// 		// Here goes the reset of the view
		// 	}
		else{
			glUniformMatrix4fv(0,1,GL_FALSE,glm::value_ptr(defaultView));
			glUniformMatrix4fv(1,1,GL_FALSE,glm::value_ptr(defaultProjection));
		}

		switch(curr_scene){
		
		// Bezier Curve
		case 1:
			// if R is clicked, it deletes all control points & curve
			if (changes.buttonPressedASCII == 82){
				cp_positions_vector.clear();

				cp_point_cpu.verts = cp_positions_vector;
				cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);

				cp_point_gpu.setVerts(cp_point_cpu.verts);
				cp_point_gpu.setCols(cp_point_cpu.cols);

				bezierCurvePoints.clear();
				bezier_cpu.verts = bezierCurvePoints;
				bezier_cpu.cols = std::vector<glm::vec3>(bezier_cpu.verts.size(), glm::vec3(0, 0, 1)); // Blue color for Bézier curve

				bezier_gpu.setVerts(bezier_cpu.verts);
				bezier_gpu.setCols(bezier_cpu.cols);
			}

			// // if P is clicked, the user places new points
			// if (changes.buttonPressedASCII == 80){
			// 	mode = 80;
			// }

			// // if M is clicked, the user can click-and-drag to move points
			// if (changes.buttonPressedASCII == 77){
			// 	mode = 77;
			// }

			// // if D is clicked, the user deletes the clicked point
			// if (changes.buttonPressedASCII == 68){
			// 	mode = 68;
			// }

			

			if (changes.clicked) {
				// if (mode == 80){
				cp_positions_vector.push_back(changes.newMouseClickLocation);	
				// }

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

		// B-Spline Curve
		case 2:
			// if R is clicked, it deletes all control points & curve
			if (changes.buttonPressedASCII == 82){
				cp_positions_vector.clear();

				cp_point_cpu.verts = cp_positions_vector;
				cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);

				cp_point_gpu.setVerts(cp_point_cpu.verts);
				cp_point_gpu.setCols(cp_point_cpu.cols);

				bSplinePoints.clear();
				bSpline_cpu.verts = bSplinePoints;
				bSpline_cpu.cols = std::vector<glm::vec3>(bSpline_cpu.verts.size(), glm::vec3(0, 0, 1)); // Blue color for Bézier curve

				bSpline_gpu.setVerts(bSpline_cpu.verts);
				bSpline_gpu.setCols(bSpline_cpu.cols);
			}

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

		// Surface of revolution
		case 3:
			// // Update B-Spline curve in GPU
			// bSplinePoints = generateQuadraticBSpline(cp_positions_vector, 4);
			bSplinePoints = generateQuadraticBSpline(test, 4);

			surfaceVertices = createSurfaceOfRevolution(bSplinePoints, 30);

			surface_cpu.verts = surfaceVertices;
			surface_cpu.cols = std::vector<glm::vec3>(surface_cpu.verts.size(), glm::vec3(0, 0, 0));

			surface_gpu.setVerts(surface_cpu.verts);
			surface_gpu.setCols(surface_cpu.cols);
			
			if (changes.isWireframe){
				// This turns ON wireframe
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
			}
			else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			
			surface_gpu.bind();
			glDrawArrays(GL_TRIANGLES, 0, surface_cpu.verts.size());

			break;
		
		// Tensor product surface #1
		case 4:
			makeTensorVertices(TensorProductSurface1, tensorVertices);

			tensor_cpu.verts = tensorVertices;
			tensor_cpu.cols = std::vector<glm::vec3>(tensorVertices.size(), glm::vec3(0.f,0.f,0.f));

			tensor_gpu.setVerts(tensor_cpu.verts);
			tensor_gpu.setCols(tensor_cpu.cols);

			if (changes.isWireframe){
				// This turns ON wireframe
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
			}
			else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			tensor_gpu.bind();

			glDrawArrays(GL_TRIANGLES, 0, tensor_cpu.verts.size());

			break;

		// Tensor product surface #2
		case 5:
			makeTensorVertices(TensorProductSurface2, tensorVertices);

			tensor_cpu.verts = tensorVertices;
			tensor_cpu.cols = std::vector<glm::vec3>(tensorVertices.size(), glm::vec3(0.f,0.f,0.f));

			tensor_gpu.setVerts(tensor_cpu.verts);
			tensor_gpu.setCols(tensor_cpu.cols);

			if (changes.isWireframe){
				// This turns ON wireframe
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
			}
			else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			tensor_gpu.bind();

			glDrawArrays(GL_TRIANGLES, 0, tensor_cpu.verts.size());

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