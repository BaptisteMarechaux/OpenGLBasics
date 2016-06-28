#version 150

uniform sampler2D u_sampler;

in VertexData {
    vec4 v_color;
    vec3 v_normal;
    vec2 v_texcoords;
} IN;

// Vecteur VERS la source lumineuse
const vec3 L = normalize(vec3(1.0, 1.0, 0.0));

out vec4 Fragment;

void main(void)
{
    vec3 N = normalize(IN.v_normal);

    //vec4 texColor = texture(u_sampler, IN.texcoords);
    vec4 Ambient = IN.v_color;

	// Equation de Lambert - calcul de l'intensite de la reflexion diffuse
	// I = cosA avec A = angle(L,N)
	float NdotL = max(dot(L, N), 0.0);
	vec4 Diffuse = vec4(vec3(NdotL), 1.0);

	

    Fragment = Ambient + Diffuse;
}