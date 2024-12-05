//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitCube.h"


struct Parameters{
	bool isPlaying =true;
	float playbackSpeed = 1.f;
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:

	// Camera is initialized to start looking at an angle, which is why the planets look like in the wrong position
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		// Spacebar toggles between Pause/Play
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
			this->UserInput.isPlaying = !this->UserInput.isPlaying;
		}

		// Left and right arrow keys decrease/increase playback speed
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
			this->UserInput.playbackSpeed -= 0.1;
		}

		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
			this->UserInput.playbackSpeed += 0.1;
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "camera");
		glm::vec3 cameraPos = camera.getPos();
		glUniform3fv(location, 1, glm::value_ptr(cameraPos));
		// std::cout << "camera.getPos(): " << "( " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}
	Camera camera;

	glm::mat4 getView(){
		return camera.getView();
	}

	Parameters getUserInput(){
		return this->UserInput;
	}

private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;

	Parameters UserInput;
};

std::vector<glm::vec3> createSurfaceOfRevolution(const std::vector<glm::vec3>& CurvePoints, std::vector<glm::vec2>& TexCoords, int numSlices) {
    std::vector<glm::vec3> surfaceVertices;
    float angleStep = glm::radians(360.0f / numSlices);

	float uStep = (1.f / float(numSlices)); 		// 1/24 in the horizontal 
	float vStep = (1.f / float(numSlices)) * 2.f; 	// 1/12 in the vertical

	float u = 1;
	float v = 0;

    // For each point in the original curve
    for (size_t i = 0; i < CurvePoints.size() - 1; ++i) {
        glm::vec3 bottom1 = CurvePoints[i];
        glm::vec3 bottom2 = CurvePoints[i + 1];

		float v2 = v + vStep;

        // Create triangles for each slice around the revolution axis
        for (int slice = 0; slice < numSlices; ++slice) {
            float angle1 = slice * angleStep;
            float angle2 = (slice + 1) * angleStep;

            // Rotate points around Y-axis
            glm::vec3 p1(bottom1.x * cos(angle1), bottom1.y, bottom1.x * sin(angle1));
            glm::vec3 p2(bottom1.x * cos(angle2), bottom1.y, bottom1.x * sin(angle2));
            glm::vec3 p3(bottom2.x * cos(angle1), bottom2.y, bottom2.x * sin(angle1));
            glm::vec3 p4(bottom2.x * cos(angle2), bottom2.y, bottom2.x * sin(angle2));

            // First triangle of the quad
            surfaceVertices.push_back(p1);
            surfaceVertices.push_back(p2);
            surfaceVertices.push_back(p3);

            // Second triangle of the quad
            surfaceVertices.push_back(p3);
            surfaceVertices.push_back(p2);
            surfaceVertices.push_back(p4);

			// Now Texture coordinates
			float u2 = u - uStep;

			TexCoords.push_back(glm::vec2(u, v));
			TexCoords.push_back(glm::vec2(u2, v));
			TexCoords.push_back(glm::vec2(u, v2));

			TexCoords.push_back(glm::vec2(u, v2));
			TexCoords.push_back(glm::vec2(u2, v));
			TexCoords.push_back(glm::vec2(u2, v2));
			
			u -= uStep;
        }

		u = 1;
		v += vStep;
    }
    return surfaceVertices;
}


struct CelestialBody{
	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 basePosition = {0.f,0.f,0.f};
	glm::vec3 axis_tilt = {0.f,0.f,0.f};

	glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f,1.f,1.f));
	glm::mat4 tilt = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.f,0.f,1.f));
	glm::mat4 planetRotation = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.f,1.f,0.f));
	glm::mat4 translation = glm::translate(glm::mat4(1.f), basePosition);
	
	glm::mat4 orbitRotation = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.f,1.f,0.f));

	// Comes from the parent's basePosition
	glm::vec3 orbitPoint = glm::vec3(0.f,0.f,0.f);

	// Tutorial
	// glm::mat4 Mscale;
	// Mb = Rotation of axis tilt * Axis rotation (planet rotation)
	// glm::mat4 Mb = tilt * planetRotation;
	// Mo = Rotation of orbit about the inclination * Translation of p0 (p0 is the planet centre)
	// glm::mat4 Mo = orbitRotation * translation;
	// p0 = Rotation orbit * Translation from the centre of parent to the centre of local

	std::vector<CelestialBody*> children;

	CelestialBody(std::string texturePath, GLint interpolation, std::vector<glm::vec3> sphere) : 
		texture(texturePath, interpolation){
			cgeom.verts = sphere;
			cgeom.normals = sphere;

			ggeom.setVerts(sphere);
			ggeom.setNormals(sphere);

	}

	void setPosition(glm::vec3 newPos){
		translation = glm::translate(glm::mat4(1.f), newPos);
	}

	void setScale(float newScale){
		scale = glm::scale(glm::mat4(1.f), glm::vec3(newScale, newScale, newScale));
	}

	void setTilt(float angle){
		tilt = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f,0.f,1.f));
	}

	void setRotation(float angle){
		planetRotation = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f,1.f,0.f));
	}

	void setOrbitRotation(float angle){
		orbitRotation = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f,1.f,0.f));
	}

	void setOrbitPoint(glm::vec3 point){
		orbitPoint = point;
	}

	glm::mat4 getLocalMatrix(){
		return orbitRotation * translation * tilt * planetRotation * scale;
	}

	void updateChildren(float tSim){
		// runs after the parent has been updated to update the children, and then its children, and so on
		// for now, just check if parent works
		return;
		// for (int i = 0; i < children.size(); i++){
		// 	children.at(i)->updateLocal(tDelta);
		// }
	}

	void updateLocal(float tSim){
		// update the local transformation matrix(ces)
		planetRotation = glm::rotate(planetRotation, glm::radians(tSim * 3.6f), glm::vec3(0.f, 1.f, 0.f));
		orbitRotation = glm::rotate(orbitRotation, glm::radians(tSim * 3.6f), glm::vec3(0.f, 1.f, 0.f));


		basePosition = (orbitRotation * glm::vec4(basePosition, 1.f));
		translation = glm::translate(glm::mat4(1.0f), glm::vec3(orbitRotation * glm::vec4(basePosition, 1.f)));

		// after the local has been done, update the children
		updateChildren(tSim);
	}
	
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 4");

	GLDebug::enable();

	// CALLBACKS
	std::shared_ptr<Assignment4> a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	UnitCube cube;
	cube.generateGeometry();

	float root2over2 = powf(2, 0.5) / 2.f;
	float root3over2 = powf(3, 0.5) / 2.f;
	float cos15 = (powf(3, 0.5) + 1) / (2 * powf(2, 0.5)); // 0.9659...
	float sin15 = (powf(3, 0.5) - 1) / (2 * powf(2, 0.5)); // 0.2588...

	std::vector<glm::vec3> sphereCoords = std::vector<glm::vec3>();
	sphereCoords.push_back(glm::vec3(0.f,-1.f,0.f));
	sphereCoords.push_back(glm::vec3(sin15,-cos15,0.f));
	sphereCoords.push_back(glm::vec3(0.5f,-root3over2,0.f));
	sphereCoords.push_back(glm::vec3(root2over2,-root2over2,0.f));
	sphereCoords.push_back(glm::vec3(root3over2,-0.5f,0.f));
	sphereCoords.push_back(glm::vec3(cos15,-sin15,0.f));
	sphereCoords.push_back(glm::vec3(1.f,0.f,0.f));
	sphereCoords.push_back(glm::vec3(cos15,sin15,0.f));
	sphereCoords.push_back(glm::vec3(root3over2,0.5f,0.f));
	sphereCoords.push_back(glm::vec3(root2over2,root2over2,0.f));
	sphereCoords.push_back(glm::vec3(0.5f,root3over2,0.f));
	sphereCoords.push_back(glm::vec3(sin15,cos15,0.f));
	sphereCoords.push_back(glm::vec3(0.f,1.f,0.f));

	// the number of slices is chosen based on the # of slices in the semi unit circle * 2 because it makes the subdivisions along every side equal
	std::vector<glm::vec2> SphereTexCoords;
	std::vector<glm::vec3> Sphere3D = createSurfaceOfRevolution(sphereCoords, SphereTexCoords, 24);

	CelestialBody sun = CelestialBody("textures/2k_sun.jpg", GL_LINEAR, Sphere3D);
	CelestialBody skybox = CelestialBody("textures/2k_stars.jpg", GL_LINEAR, Sphere3D);
	CelestialBody earth = CelestialBody("textures/2k_earth_daymap.jpg", GL_LINEAR, Sphere3D);
	CelestialBody moon = CelestialBody("textures/2k_moon.jpg", GL_LINEAR, Sphere3D);

	sun.cgeom.texCoords = SphereTexCoords;
	sun.ggeom.setTexCoords(sun.cgeom.texCoords);
	sun.setOrbitPoint(glm::vec3(0.f,0.f,0.f));
	sun.setOrbitRotation(0.f);
	sun.setTilt(0.f);

	skybox.cgeom.texCoords = SphereTexCoords;
	skybox.ggeom.setTexCoords(skybox.cgeom.texCoords);
	// Constants
	skybox.setScale(50.f);

	earth.cgeom.texCoords = SphereTexCoords;
	earth.ggeom.setTexCoords(earth.cgeom.texCoords);
	// Constants
	earth.setScale(0.5f);
	earth.setTilt(23.45f);

	// Variables (testing)
	earth.setPosition(glm::vec3(3.f,0.f,0.f));
	earth.setRotation(135.f);
	earth.setOrbitRotation(15.f);

	moon.cgeom.texCoords = SphereTexCoords;
	moon.ggeom.setTexCoords(moon.cgeom.texCoords);
	moon.setScale(0.1f);
	moon.setPosition(glm::vec3(3.8f,0.f,0.f));

	// Define transformation hierarchies so that the transformations are relative to parents
	sun.children.push_back(&earth);
	earth.children.push_back(&moon);

	float tprev = glfwGetTime();

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		a4->viewPipeline(shader);
		glm::mat4 viewMat = a4->getView();

		Parameters userInput = a4->getUserInput();

		float tcurr = glfwGetTime();

		float tDelta = tcurr - tprev;
		std::cout << "tDelta:" << tDelta << std::endl;
		float tSim = tDelta * userInput.playbackSpeed;
		std::cout << "tSim:" << tSim << std::endl;
		tprev = tcurr;
		sun.updateLocal(tSim);

		// Sun
		// Disable shading
		glUniform1i(glGetUniformLocation(shader, "useShading"), 0);

		sun.texture.bind();
		sun.ggeom.bind();

		GLint transformMat = glGetUniformLocation(shader, "M");

		glUniformMatrix4fv(transformMat,1,GL_FALSE, glm::value_ptr(sun.getLocalMatrix()));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sun.cgeom.verts.size()));
		sun.texture.unbind();

		// Skybox
		// Re-enable shading
		glUniform1i(glGetUniformLocation(shader, "useShading"), 1);

		skybox.texture.bind();
		skybox.ggeom.bind();

		glUniformMatrix4fv(transformMat,1,GL_FALSE, glm::value_ptr(skybox.getLocalMatrix()));
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(skybox.cgeom.verts.size()));
		skybox.texture.unbind();

		// Earth
		earth.texture.bind();
		earth.ggeom.bind();

		glUniformMatrix4fv(transformMat,1,GL_FALSE, glm::value_ptr(earth.getLocalMatrix()));
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.cgeom.verts.size()));
		earth.texture.unbind();

		// Moon
		moon.texture.bind();
		moon.ggeom.bind();

		glUniformMatrix4fv(transformMat,1,GL_FALSE, glm::value_ptr(moon.getLocalMatrix()));
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(moon.cgeom.verts.size()));
		moon.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
