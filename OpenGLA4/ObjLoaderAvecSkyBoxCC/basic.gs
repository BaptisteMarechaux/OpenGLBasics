
#version 150

// en entree: points, lines, triangles
layout(triangles) in;
// en sortie: points, line_strip, triangle_strip
layout(triangle_strip, max_vertices = 6) out;

in VertexDataIn {
    vec4 v_color;
    vec3 v_normal;
    vec2 v_texcoords;
} IN[];

out VertexData {
    vec4 v_color;
    vec3 v_normal;
    vec2 v_texcoords;
} OUT;

uniform float u_time;

void main(void)
{
	int count = gl_in.length();
	for (int index = 0; index < count; ++index) {
		gl_Position = gl_in[index].gl_Position;
		OUT.v_color = IN[index].v_color;
		OUT.v_normal = IN[index].v_normal;
		OUT.v_texcoords = IN[index].v_texcoords;
		EmitVertex();
	}
	EndPrimitive();

	for (int index = 0; index < count; ++index) {
		gl_Position = gl_in[index].gl_Position + vec4(IN[index].v_normal * 0.2 * cos(u_time), 0.0);
		OUT.v_color = vec4(1.0, 0.0, 0.0, 1.0);
		OUT.v_normal = IN[index].v_normal;
		OUT.v_texcoords = IN[index].v_texcoords;
		EmitVertex();
	}
	EndPrimitive();
}