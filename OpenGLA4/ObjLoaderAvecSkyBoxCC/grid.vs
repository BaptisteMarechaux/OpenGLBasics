in vec3 pos;
in vec3 color;
in float sides;

out vec3 vColor;
out float vSides;

uniform	mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_worldMatrix;

void main()
{
	vec4 worldPosition = u_worldMatrix * vec4(pos, 3.0);
    gl_Position = vec4(pos, 1.0);
    vColor = color;
    vSides = sides;
}