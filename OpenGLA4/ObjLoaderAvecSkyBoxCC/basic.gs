
#version 150

//layout(points) in;
//layout(line_strip, max_vertices = 64) out;

in vec3 vColor[];
in float vSides[];
out vec3 fColor;

const float PI = 3.1415926;


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


	fColor = vColor[0];

    // Safe, GLfloats can represent small integers exactly
    for (int i = 0; i <= vSides[0]; i++) {
        // Angle between each side in radians
        float ang = PI * 2.0 / vSides[0] * i;

        // Offset from center of point (0.3 to accomodate for aspect ratio)
        vec4 offset = vec4(cos(ang) * 0.3, -sin(ang) * 0.4, 0.0, 0.0);
        gl_Position = gl_in[0].gl_Position + offset;

        EmitVertex();
    }

    EndPrimitive();
}