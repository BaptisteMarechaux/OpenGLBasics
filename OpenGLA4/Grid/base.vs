#version 150

in vec2 center;
in vec3 color;
in float sides;
in float min;
in float max;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec3 g_Color;
out float g_Min;
out float g_Max;

void main()
{
	//vec4 world_pos = modelMatrix * vec4(center, 0.0, 1.0);
	//vec4 view_pos = viewMatrix * world_pos;
	//gl_Position = projectionMatrix * viewMatrix *  vec4(center, 0.0, 1.0);
	gl_Position = vec4(center, 0.0, 1.0);
    g_Color = color;
	g_Min = min;
	g_Max = max;
}