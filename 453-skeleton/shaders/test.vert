#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoord;

uniform mat4 M;

// These are for the camera; view & perspective matrices
uniform mat4 V;
uniform mat4 P;

uniform vec3 camera;

out vec3 fragPos;
out vec3 fragColor;
out vec3 n;
out vec2 tc;
out vec3 cameraPos;

void main() {
	tc = texCoord;
	fragColor = color;
	n = normal;

	// Converting to world space
	cameraPos = vec3(M * vec4(camera, 1.0));
	fragPos = vec3(M * vec4(pos, 1.0));
	gl_Position = P * V * M * vec4(pos, 1.0);
}
