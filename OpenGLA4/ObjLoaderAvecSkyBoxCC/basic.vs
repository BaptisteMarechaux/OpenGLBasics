#version 150

#extension GL_ARB_explicit_attrib_location : enable


in vec2 pos;
in vec3 color;
in float sides;

out vec3 vColor;
out float vSides;


layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoords;

uniform	mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_worldMatrix;

out VertexDataIn {
    vec4 v_color;
    vec3 v_normal;
    vec2 v_texcoords;
} OUT;

// Vecteur vers le haut de l'hemisphere
const vec3 UP = vec3(0.0, 1.0, 0.0);
// Couleurs pour l'hemisphere Ambient Lighting
const vec4 Blue = vec4(0.0, 0.0, 1.0, 1.0);
const vec4 Green = vec4(0.0, 1.0, 0.0, 1.0);

uniform float u_time;

void main(void)
{
	vec4 worldPosition = u_worldMatrix * a_position;
	vec3 N = mat3(u_worldMatrix) * a_normal;
	OUT.v_normal = N;

	//float NdotL = max(dot(L, N), 0.0);
	//worldPosition.xyz += OUT.v_normal * NdotL * abs(cos(u_time));

	float ambientFactor = (dot(N, UP) + 1.0) / 2.0;
	OUT.v_color = mix(Green, Blue, ambientFactor);

	OUT.v_texcoords = a_texcoords;
	gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;


	//gl_Position = vec4(pos, 0.0, 1.0);
    vColor = color;
    vSides = sides;
}