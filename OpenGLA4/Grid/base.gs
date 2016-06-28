#version 150

layout(points) in;
layout(line_strip, max_vertices = 101) out;

in vec3 g_Color[];
in float g_Min[];
in float g_Max[];
out vec3 fColor;

void main()
{
    fColor = g_Color[0];

	for (float j = g_Min[0]; j <= g_Max[0]; j+=0.1f) {
		vec4 offset = vec4(j, -1.0f, 0.0f, 0.0);
		gl_Position = gl_in[0].gl_Position + offset;
		EmitVertex();

		offset = vec4(j, 1.0f, 0.0f, 0.0f);
		gl_Position = gl_in[0].gl_Position + offset;
		EmitVertex();

		EndPrimitive();

		offset = vec4(-1.0f, j, 0.0f, 0.0f);
		gl_Position = gl_in[0].gl_Position + offset;
		EmitVertex();

		offset = vec4(1.0f, j, 0.0f, 0.0f);
		gl_Position = gl_in[0].gl_Position + offset;
		EmitVertex();

		EndPrimitive();
	}
}