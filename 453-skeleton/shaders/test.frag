#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;
in vec2 tc;
in vec3 cameraPos;

uniform sampler2D sampler;
uniform bool useShading;

out vec4 color;

void main() {
	vec4 d = texture(sampler, tc);
	if (d.a < 0.01){
		discard;
	}

	if (!useShading) {
        // No shading; just use the texture color
        color = vec4(vec3(d), d.a);
        return;
    }

	// Normalize the vector (have to do)
	vec3 norm = normalize(n);
	
	vec3 lightPosition = vec3(0.0, 0.0, 0.0);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// Diffuse Lighting
	vec3 lightDirection = normalize(lightPosition - fragPos);
	vec3 diffuse = lightColor * max(dot(norm, lightDirection), 0.0);

	// Specular Lighting
	float ks = 0.5;
	float alpha = 32;
	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 r = reflect(-lightDirection, norm);
	vec3 specular = ks * pow(max(dot(norm, r), 0), alpha) * lightColor;

	// Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

	vec3 phong = diffuse + ambient + specular;
	vec3 phongWithTexture = phong * vec3(d);

	color = vec4(phongWithTexture, d.a);
}
