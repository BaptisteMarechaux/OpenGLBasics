#version 150

uniform samplerCube u_cubemap;

// Interface block (bloc d'interface)
in VertexData {
    vec3 v_position;
    vec3 v_texcoords;
} IN;

out vec4 Fragment;

void main(void)
{
    Fragment = texture(u_cubemap, IN.v_texcoords);
}