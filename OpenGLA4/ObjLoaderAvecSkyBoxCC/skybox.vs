#version 150

#extension GL_ARB_explicit_attrib_location : enable

layout(location = 0) in vec3 a_position;

uniform	mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

// Interface block (bloc d'interface)
out VertexData {
    vec3 v_position;
    vec3 v_texcoords;
} OUT;

void main(void)
{
	OUT.v_position = a_position;
	OUT.v_texcoords = a_position;
	// Ici on ANNULE la translation de la cam
	vec4 pos = u_projectionMatrix * u_viewMatrix 
				* vec4(a_position, 0.0);

	gl_Position = pos.xyzz;

}