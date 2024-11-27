#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;
in vec2 tc;

//uniform vec3 lightPosition;
uniform sampler2D sampler;

out vec4 color;

void main() {
	vec4 d = texture(sampler, tc);
	if (d.a < 0.01)
	discard;
	color = d;
}
